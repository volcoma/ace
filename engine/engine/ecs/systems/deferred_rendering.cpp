#include "deferred_rendering.h"
#include <engine/assets/asset_manager.h>
#include <engine/ecs/components/camera_component.h>
#include <engine/ecs/components/light_component.h>
#include <engine/ecs/components/model_component.h>
#include <engine/ecs/components/reflection_probe_component.h>
#include <engine/ecs/components/transform_component.h>
#include <engine/events.h>
#include <engine/rendering/camera.h>
#include <engine/rendering/material.h>
#include <engine/rendering/mesh.h>
#include <engine/rendering/model.h>
#include <engine/rendering/renderer.h>

#include <graphics/index_buffer.h>
#include <graphics/render_pass.h>
#include <graphics/render_view.h>
#include <graphics/texture.h>
#include <graphics/vertex_buffer.h>

namespace ace
{

bool update_lod_data(lod_data& data,
                     const std::vector<urange32_t>& lod_limits,
                     std::size_t total_lods,
                     float transition_time,
                     float dt,
                     const asset_handle<mesh>& mesh,
                     const math::transform& world,
                     const camera& cam)
{
    if(!mesh)
        return false;

    if(total_lods <= 1)
        return true;

    const auto& viewport = cam.get_viewport_size();
    irect32_t rect = mesh.get().calculate_screen_rect(world, cam);

    float percent = math::clamp((float(rect.height()) / float(viewport.height)) * 100.0f, 0.0f, 100.0f);

    std::size_t lod = 0;
    for(size_t i = 0; i < lod_limits.size(); ++i)
    {
        const auto& range = lod_limits[i];
        if(range.contains(urange32_t::value_type(percent)))
        {
            lod = i;
        }
    }

    lod = math::clamp<std::size_t>(lod, 0, total_lods - 1);
    if(data.target_lod_index != lod && data.target_lod_index == data.current_lod_index)
        data.target_lod_index = static_cast<std::uint32_t>(lod);

    if(data.current_lod_index != data.target_lod_index)
        data.current_time += dt;

    if(data.current_time >= transition_time)
    {
        data.current_lod_index = data.target_lod_index;
        data.current_time = 0.0f;
    }

    if(percent < 1.0f)
        return false;

    return true;
}

auto should_rebuild_reflections(visibility_set_models_t& visibility_set, const reflection_probe& probe) -> bool
{
    if(probe.method == reflect_method::environment)
        return true;

    for(auto& element : visibility_set)
    {
        auto& transform_comp_ref = element.get<transform_component>();
        auto& model_comp_ref = element.get<model_component>();

        const auto& model = model_comp_ref.get_model();
        if(!model.is_valid())
            continue;

        const auto lod = model.get_lod(0);
        if(!lod)
        {
            continue;
        }

        const auto& mesh = lod.get();

        const auto& world_transform = transform_comp_ref.get_transform_global();

        const auto& bounds = mesh.get_bounds();

        bool result = false;

        for(std::uint32_t i = 0; i < 6; ++i)
        {
            const auto& frustum = camera::get_face_camera(i, world_transform).get_frustum();
            result |= math::frustum::test_obb(frustum, bounds, world_transform);
        }

        if(result)
            return true;
    }

    return false;
}

auto should_rebuild_shadows(visibility_set_models_t& visibility_set, const light&) -> bool
{
    for(auto& element : visibility_set)
    {
        auto& transform_comp_ref = element.get<transform_component>();
        auto& model_comp_ref = element.get<model_component>();

        const auto& model = model_comp_ref.get_model();
        if(!model.is_valid())
            continue;

        const auto lod = model.get_lod(0);
        if(!lod)
        {
            continue;
        }

        const auto& mesh = lod.get();
        const auto& world_transform = transform_comp_ref.get_transform_global();
        const auto& bounds = mesh.get_bounds();

        bool result = false;

        // for(std::uint32_t i = 0; i < 6; ++i)
        //{
        //	const auto& frustum = camera::get_face_camera(i, world_transform).get_frustum();
        //	result |= math::frustum::test_obb(frustum, bounds, world_transform);
        //}

        if(result)
            return true;
    }

    return false;
}

void deferred_rendering::on_frame_render(rtti::context& ctx, delta_t dt)
{
    auto& ec = ctx.get<ecs>();

    build_reflections_pass(ec, dt);
    build_shadows_pass(ec, dt);
}

void deferred_rendering::build_reflections_pass(ecs& ec, delta_t dt)
{
    auto query = visibility_query::dirty | visibility_query::fixed | visibility_query::reflection_caster;

    auto dirty_models = gather_visible_models(ec, nullptr, query);
    ec.get_scene().view<transform_component, reflection_probe_component>().each(
        [&](auto e, auto&& transform_comp, auto&& reflection_probe_comp)
        {
            entt::handle entity(ec.get_scene(), e);

            const auto& world_tranform = transform_comp.get_transform_global();
            const auto& probe = reflection_probe_comp.get_probe();

            auto cubemap_fbo = reflection_probe_comp.get_cubemap_fbo();
            bool should_rebuild = true;

            //            if(!transform_comp.is_touched() && !reflection_probe_comp.is_touched())
            {
                // If reflections shouldn't be rebuilt - continue.
                should_rebuild = should_rebuild_reflections(dirty_models, probe);
            }

            if(!should_rebuild)
                return;

            {
                // iterate trough each cube face
                for(std::uint32_t face = 0; face < 6; ++face)
                {
                    auto camera = camera::get_face_camera(face, world_tranform);
                    camera.set_far_clip(reflection_probe_comp.get_probe().box_data.extents.r);
                    auto& render_view = reflection_probe_comp.get_render_view(face);
                    camera.set_viewport_size(usize32_t(cubemap_fbo->get_size()));
                    auto& camera_lods = lod_data_[entity];
                    visibility_set_models_t visibility_set;

                    if(probe.method != reflect_method::environment)
                    {
                        auto query = visibility_query::fixed | visibility_query::reflection_caster;
                        if(!should_rebuild)
                        {
                            query |= visibility_query::dirty;
                        }

                        visibility_set = gather_visible_models(ec, &camera, query);
                    }

                    gfx::frame_buffer::ptr output = nullptr;
                    output = g_buffer_pass(output, camera, render_view, visibility_set, camera_lods, dt);
                    output = lighting_pass(output, camera, render_view, ec, dt);
                    output = atmospherics_pass(output, camera, render_view, ec, dt);
                    output = tonemapping_pass(output, camera, render_view);

                    gfx::render_pass pass_fill("cubemap_fill");
                    pass_fill.bind(cubemap_fbo.get());
                    pass_fill.touch();
                    gfx::blit(pass_fill.id,
                              cubemap_fbo->get_texture()->native_handle(),
                              0,
                              0,
                              0,
                              uint16_t(face),
                              output->get_texture()->native_handle());
                }
            }
        });
}

void deferred_rendering::build_shadows_pass(ecs& ec, delta_t dt)
{
    auto query = visibility_query::dirty | visibility_query::fixed | visibility_query::shadow_caster;

    auto dirty_models = gather_visible_models(ec, nullptr, query);

    ec.get_scene().view<transform_component, light_component>().each(
        [&](auto e, auto&& transform_comp, auto&& light_comp)
        {
            // const auto& world_tranform = transform_comp.get_transform();
            const auto& light = light_comp.get_light();

            bool should_rebuild = true;

            //            if(!transform_comp.is_touched() && !light_comp.is_touched())
            {
                // If shadows shouldn't be rebuilt - continue.
                should_rebuild = should_rebuild_shadows(dirty_models, light);
            }

            if(!should_rebuild)
                return;
        });
}

auto deferred_rendering::render_models(camera& camera,
                                       gfx::render_view& render_view,
                                       ecs& ec,
                                       const visibility_set_models_t& visibility_set,
                                       lod_data_container& camera_lods,
                                       delta_t dt) -> gfx::frame_buffer::ptr
{
    gfx::frame_buffer::ptr output = nullptr;

    output = g_buffer_pass(output, camera, render_view, visibility_set, camera_lods, dt);

    output = reflection_probe_pass(output, camera, render_view, ec, dt);

    output = lighting_pass(output, camera, render_view, ec, dt);

    output = atmospherics_pass(output, camera, render_view, ec, dt);

    output = tonemapping_pass(output, camera, render_view);

    return output;
}

auto deferred_rendering::g_buffer_pass(gfx::frame_buffer::ptr input,
                                       camera& camera,
                                       gfx::render_view& render_view,
                                       const visibility_set_models_t& visibility_set,
                                       lod_data_container& camera_lods,
                                       delta_t dt) -> gfx::frame_buffer::ptr
{
    const auto& view = camera.get_view();
    const auto& proj = camera.get_projection();
    const auto& viewport_size = camera.get_viewport_size();
    auto g_buffer_fbo = render_view.get_g_buffer_fbo(viewport_size);
    gfx::render_pass pass("g_buffer_fill");
    pass.clear();
    pass.set_view_proj(view, proj);
    pass.bind(g_buffer_fbo.get());

    for(auto& e : visibility_set)
    {
        auto& transform_comp = e.get<transform_component>();
        auto& model_comp = e.get<model_component>();

        const auto& model = model_comp.get_model();
        if(!model.is_valid())
            continue;

        const auto& world_transform = transform_comp.get_transform_global();
        const auto clip_planes = math::vec2(camera.get_near_clip(), camera.get_far_clip());

        auto& lod_data = camera_lods[e];
        const auto transition_time = model.get_lod_transition_time();
        const auto lod_count = model.get_lods().size();
        const auto& lod_limits = model.get_lod_limits();
        const auto current_time = lod_data.current_time;
        const auto current_lod_index = lod_data.current_lod_index;
        const auto target_lod_index = lod_data.target_lod_index;

        const auto current_mesh = model.get_lod(current_lod_index);
        if(!current_mesh)
            continue;

        if(false == update_lod_data(lod_data,
                                    lod_limits,
                                    lod_count,
                                    transition_time,
                                    dt.count(),
                                    current_mesh,
                                    world_transform,
                                    camera))
            continue;
        const auto params = math::vec3{0.0f, -1.0f, (transition_time - current_time) / transition_time};

        const auto params_inv = math::vec3{1.0f, 1.0f, current_time / transition_time};

        const auto& bone_transforms = model_comp.get_bone_transforms();

        model.render(pass.id,
                     world_transform,
                     bone_transforms,
                     true,
                     true,
                     true,
                     0,
                     current_lod_index,
                     geom_program_.get(),
                     geom_skinned_program_.get(),
                     [&camera, &clip_planes, &params](auto& p)
                     {
                         auto camera_pos = camera.get_position();
                         p.set_uniform("u_camera_wpos", camera_pos);
                         p.set_uniform("u_camera_clip_planes", clip_planes);
                         p.set_uniform("u_lod_params", params);
                     });

        if(current_time != 0.0f)
        {
            model.render(pass.id,
                         world_transform,
                         bone_transforms,
                         true,
                         true,
                         true,
                         0,
                         target_lod_index,
                         geom_program_.get(),
                         geom_skinned_program_.get(),
                         [&params_inv](auto& p)
                         {
                             p.set_uniform("u_lod_params", params_inv);
                         });
        }
    }
    gfx::discard();

    return g_buffer_fbo;
}

auto deferred_rendering::lighting_pass(gfx::frame_buffer::ptr input,
                                       camera& camera,
                                       gfx::render_view& render_view,
                                       ecs& ec,
                                       delta_t dt) -> gfx::frame_buffer::ptr
{
    const auto& view = camera.get_view();
    const auto& proj = camera.get_projection();

    const auto& viewport_size = camera.get_viewport_size();
    auto g_buffer_fbo = render_view.get_g_buffer_fbo(viewport_size).get();

    static auto light_buffer_format =
        gfx::get_best_format(BGFX_CAPS_FORMAT_TEXTURE_FRAMEBUFFER,
                             gfx::format_search_flags::four_channels | gfx::format_search_flags::requires_alpha |
                                 gfx::format_search_flags::half_precision_float);

    auto light_buffer =
        render_view.get_texture("LBUFFER", viewport_size.width, viewport_size.height, false, 1, light_buffer_format);
    auto l_buffer_fbo = render_view.get_fbo("LBUFFER", {light_buffer});
    const auto buffer_size = l_buffer_fbo->get_size();

    gfx::render_pass pass("light_buffer_fill");
    pass.bind(l_buffer_fbo.get());
    pass.set_view_proj(view, proj);
    pass.clear(BGFX_CLEAR_COLOR, 0, 0.0f, 0);
    auto refl_buffer =
        render_view.get_texture("RBUFFER", viewport_size.width, viewport_size.height, false, 1, light_buffer_format)
            .get();

    ec.get_scene().view<transform_component, light_component>().each(
        [&](auto e, auto&& transform_comp_ref, auto&& light_comp_ref)
        {
            const auto& light = light_comp_ref.get_light();
            const auto& world_transform = transform_comp_ref.get_transform_global();
            const auto& light_position = world_transform.get_position();
            const auto& light_direction = world_transform.z_unit_axis();

            irect32_t rect(0, 0, irect32_t::value_type(buffer_size.width), irect32_t::value_type(buffer_size.height));
            if(light_comp_ref.compute_projected_sphere_rect(rect, light_position, light_direction, view, proj) == 0)
                return;

            gpu_program* program = nullptr;
            if(light.type == light_type::directional && directional_light_program_)
            {
                // Draw light.
                program = directional_light_program_.get();
                program->begin();
                program->set_uniform("u_light_direction", light_direction);
            }
            if(light.type == light_type::point && point_light_program_)
            {
                float light_data[4] = {light.point_data.range, light.point_data.exponent_falloff, 0.0f, 0.0f};

                // Draw light.
                program = point_light_program_.get();
                program->begin();
                program->set_uniform("u_light_position", light_position);
                program->set_uniform("u_light_data", light_data);
            }

            if(light.type == light_type::spot && spot_light_program_)
            {
                float light_data[4] = {light.spot_data.get_range(),
                                       math::cos(math::radians(light.spot_data.get_inner_angle() * 0.5f)),
                                       math::cos(math::radians(light.spot_data.get_outer_angle() * 0.5f)),
                                       0.0f};

                // Draw light.
                program = spot_light_program_.get();
                program->begin();
                program->set_uniform("u_light_position", light_position);
                program->set_uniform("u_light_direction", light_direction);
                program->set_uniform("u_light_data", light_data);
            }

            if(program)
            {
                float light_color_intensity[4] = {light.color.value.r,
                                                  light.color.value.g,
                                                  light.color.value.b,
                                                  light.intensity};
                auto camera_pos = camera.get_position();
                program->set_uniform("u_light_color_intensity", light_color_intensity);
                program->set_uniform("u_camera_position", camera_pos);
                program->set_texture(0, "s_tex0", g_buffer_fbo->get_texture(0).get());
                program->set_texture(1, "s_tex1", g_buffer_fbo->get_texture(1).get());
                program->set_texture(2, "s_tex2", g_buffer_fbo->get_texture(2).get());
                program->set_texture(3, "s_tex3", g_buffer_fbo->get_texture(3).get());
                program->set_texture(4, "s_tex4", g_buffer_fbo->get_texture(4).get());
                program->set_texture(5, "s_tex5", refl_buffer);
                program->set_texture(6, "s_tex6", ibl_brdf_lut_.get_ptr().get());

                gfx::set_scissor(rect.left, rect.top, rect.width(), rect.height());
                auto topology = gfx::clip_quad(1.0f);
                gfx::set_state(topology | BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_BLEND_ADD);
                gfx::submit(pass.id, program->native_handle());
                gfx::set_state(BGFX_STATE_DEFAULT);

                program->end();
            }
        });

    gfx::discard();

    return l_buffer_fbo;
}

auto deferred_rendering::reflection_probe_pass(gfx::frame_buffer::ptr input,
                                               camera& camera,
                                               gfx::render_view& render_view,
                                               ecs& ec,
                                               delta_t dt) -> gfx::frame_buffer::ptr
{
    const auto& view = camera.get_view();
    const auto& proj = camera.get_projection();

    const auto& viewport_size = camera.get_viewport_size();
    auto g_buffer_fbo = render_view.get_g_buffer_fbo(viewport_size).get();

    static auto refl_buffer_format =
        gfx::get_best_format(BGFX_CAPS_FORMAT_TEXTURE_FRAMEBUFFER,
                             gfx::format_search_flags::four_channels | gfx::format_search_flags::requires_alpha |
                                 gfx::format_search_flags::half_precision_float);

    auto refl_buffer =
        render_view.get_texture("RBUFFER", viewport_size.width, viewport_size.height, false, 1, refl_buffer_format);
    auto r_buffer_fbo = render_view.get_fbo("RBUFFER", {refl_buffer});
    const auto buffer_size = refl_buffer->get_size();

    gfx::render_pass pass("refl_buffer_fill");
    pass.bind(r_buffer_fbo.get());
    pass.set_view_proj(view, proj);
    pass.clear(BGFX_CLEAR_COLOR, 0, 0.0f, 0);

    ec.get_scene().view<transform_component, reflection_probe_component>().each(
        [&](auto e, auto&& transform_comp_ref, auto&& probe_comp_ref)
        {
            const auto& probe = probe_comp_ref.get_probe();
            const auto& world_transform = transform_comp_ref.get_transform_global();
            const auto& probe_position = world_transform.get_position();

            irect32_t rect(0, 0, irect32_t::value_type(buffer_size.width), irect32_t::value_type(buffer_size.height));
            if(probe_comp_ref.compute_projected_sphere_rect(rect, probe_position, view, proj) == 0)
                return;

            const auto cubemap = probe_comp_ref.get_cubemap();

            gpu_program* program = nullptr;
            float influence_radius = 0.0f;
            if(probe.type == probe_type::sphere && sphere_ref_probe_program_)
            {
                program = sphere_ref_probe_program_.get();
                program->begin();
                influence_radius = probe.sphere_data.range;
            }

            if(probe.type == probe_type::box && box_ref_probe_program_)
            {
                math::transform t;
                t.set_scale(probe.box_data.extents);
                t = world_transform * t;
                auto u_inv_world = math::inverse(t).get_matrix();
                float data2[4] = {probe.box_data.extents.x,
                                  probe.box_data.extents.y,
                                  probe.box_data.extents.z,
                                  probe.box_data.transition_distance};

                program = box_ref_probe_program_.get();
                program->begin();
                program->set_uniform("u_inv_world", math::value_ptr(u_inv_world));
                program->set_uniform("u_data2", data2);

                influence_radius = math::length(t.get_scale() + probe.box_data.transition_distance);
            }

            if(program)
            {
                float mips = cubemap ? float(cubemap->info.numMips) : 1.0f;
                float data0[4] = {
                    probe_position.x,
                    probe_position.y,
                    probe_position.z,
                    influence_radius,
                };

                float data1[4] = {mips, 0.0f, 0.0f, 0.0f};

                program->set_uniform("u_data0", data0);
                program->set_uniform("u_data1", data1);

                program->set_texture(0, "s_tex0", g_buffer_fbo->get_texture(0).get());
                program->set_texture(1, "s_tex1", g_buffer_fbo->get_texture(1).get());
                program->set_texture(2, "s_tex2", g_buffer_fbo->get_texture(2).get());
                program->set_texture(3, "s_tex3", g_buffer_fbo->get_texture(3).get());
                program->set_texture(4, "s_tex4", g_buffer_fbo->get_texture(4).get());
                program->set_texture(5, "s_tex_cube", cubemap.get());
                gfx::set_scissor(rect.left, rect.top, rect.width(), rect.height());
                auto topology = gfx::clip_quad(1.0f);
                gfx::set_state(topology | BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_BLEND_ALPHA);
                gfx::submit(pass.id, program->native_handle());
                gfx::set_state(BGFX_STATE_DEFAULT);
                program->end();
            }
        });

    gfx::discard();

    return r_buffer_fbo;
}

auto deferred_rendering::atmospherics_pass(gfx::frame_buffer::ptr input,
                                           camera& camera,
                                           gfx::render_view& render_view,
                                           ecs& ec,
                                           delta_t dt) -> gfx::frame_buffer::ptr
{
    const auto& viewport_size = camera.get_viewport_size();
    static auto light_buffer_format =
        gfx::get_best_format(BGFX_CAPS_FORMAT_TEXTURE_FRAMEBUFFER,
                             gfx::format_search_flags::four_channels | gfx::format_search_flags::requires_alpha |
                                 gfx::format_search_flags::half_precision_float);

    auto light_buffer = render_view.get_texture("LBUFFER",
                                                viewport_size.width,
                                                viewport_size.height,
                                                false,
                                                1,
                                                light_buffer_format,
                                                gfx::get_default_rt_sampler_flags());
    input = render_view.get_fbo("LBUFFER", {light_buffer, render_view.get_depth_buffer(viewport_size)});

    atmospheric_pass::run_params params;
    atmospheric_pass_perez::run_params params_perez;

    bool found_sun = false;
    auto light_direction = math::normalize(math::vec3(0.2f, -0.8f, 1.0f));

    ec.get_scene().view<transform_component, light_component>().each(
        [&](auto e, auto&& transform_comp_ref, auto&& light_comp_ref)
        {
            if(found_sun)
            {
                return;
            }

            const auto& light = light_comp_ref.get_light();

            if(light.type == light_type::directional)
            {
                found_sun = true;
                const auto& world_transform = transform_comp_ref.get_transform_global();
                params.light_direction = world_transform.z_unit_axis();

                params_perez.light_direction = world_transform.z_unit_axis();
            }
        });

    return atmospheric_pass_.run(input, camera, dt, params);

    // return atmospheric_pass_perez_.run(input, camera, dt, params_perez);
}

auto deferred_rendering::tonemapping_pass(gfx::frame_buffer::ptr input, camera& camera, gfx::render_view& render_view)
    -> gfx::frame_buffer::ptr
{
    if(!input)
        return nullptr;

    const auto& viewport_size = camera.get_viewport_size();
    auto surface = render_view.get_output_fbo(viewport_size);
    const auto output_size = surface->get_size();
    const auto& view = camera.get_view();
    const auto& proj = camera.get_projection();
    gfx::render_pass pass("output_buffer_fill");
    pass.set_view_proj(view, proj);
    pass.bind(surface.get());

    if(surface && gamma_correction_program_)
    {
        gamma_correction_program_->begin();
        gamma_correction_program_->set_texture(0, "s_input", input->get_texture().get());
        irect32_t rect(0, 0, irect32_t::value_type(output_size.width), irect32_t::value_type(output_size.height));
        gfx::set_scissor(rect.left, rect.top, rect.width(), rect.height());
        auto topology = gfx::clip_quad(1.0f);
        gfx::set_state(topology | BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A);
        gfx::submit(pass.id, gamma_correction_program_->native_handle());
        gfx::set_state(BGFX_STATE_DEFAULT);
        gamma_correction_program_->end();
    }

    gfx::discard();

    return surface;
}

// void deferred_rendering::receive(entity e)
//{
//     lod_data_.erase(e);
//     for(auto& pair : lod_data_)
//     {
//         pair.second.erase(e);
//     }
// }
deferred_rendering::deferred_rendering()
{
}

deferred_rendering::~deferred_rendering()
{
}

auto deferred_rendering::init(rtti::context& ctx) -> bool
{
    APPLOG_INFO("{}::{}", hpp::type_name_str(*this), __func__);

    auto& ev = ctx.get<events>();
    ev.on_frame_render.connect(sentinel_, 1000, this, &deferred_rendering::on_frame_render);

    auto& am = ctx.get<asset_manager>();

    auto vs_clip_quad = am.load<gfx::shader>("engine:/data/shaders/vs_clip_quad.sc");
    auto fs_deferred_point_light = am.load<gfx::shader>("engine:/data/shaders/fs_deferred_point_light.sc");
    auto fs_deferred_spot_light = am.load<gfx::shader>("engine:/data/shaders/fs_deferred_spot_light.sc");
    auto fs_deferred_directional_light = am.load<gfx::shader>("engine:/data/shaders/fs_deferred_directional_light.sc");
    auto fs_gamma_correction = am.load<gfx::shader>("engine:/data/shaders/fs_gamma_correction.sc");
    auto fs_sphere_reflection_probe = am.load<gfx::shader>("engine:/data/shaders/fs_sphere_reflection_probe.sc");
    auto fs_box_reflection_probe = am.load<gfx::shader>("engine:/data/shaders/fs_box_reflection_probe.sc");
    auto vs_clip_quad_ex = am.load<gfx::shader>("engine:/data/shaders/vs_clip_quad_ex.sc");

    auto vs_deferred_geom = am.load<gfx::shader>("engine:/data/shaders/vs_deferred_geom.sc");
    auto vs_deferred_geom_skinned = am.load<gfx::shader>("engine:/data/shaders/vs_deferred_geom_skinned.sc");
    auto fs_deferred_geom = am.load<gfx::shader>("engine:/data/shaders/fs_deferred_geom.sc");

    geom_program_ = std::make_unique<gpu_program>(vs_deferred_geom, fs_deferred_geom);
    geom_skinned_program_ = std::make_unique<gpu_program>(vs_deferred_geom_skinned, fs_deferred_geom);

    ibl_brdf_lut_ = am.load<gfx::texture>("engine:/data/textures/ibl_brdf_lut.png");

    point_light_program_ = std::make_unique<gpu_program>(vs_clip_quad, fs_deferred_point_light);

    spot_light_program_ = std::make_unique<gpu_program>(vs_clip_quad, fs_deferred_spot_light);

    directional_light_program_ = std::make_unique<gpu_program>(vs_clip_quad, fs_deferred_directional_light);

    gamma_correction_program_ = std::make_unique<gpu_program>(vs_clip_quad, fs_gamma_correction);

    sphere_ref_probe_program_ = std::make_unique<gpu_program>(vs_clip_quad_ex, fs_sphere_reflection_probe);

    box_ref_probe_program_ = std::make_unique<gpu_program>(vs_clip_quad_ex, fs_box_reflection_probe);

    atmospheric_pass_.init(ctx);
    atmospheric_pass_perez_.init(ctx);

    return true;
}

auto deferred_rendering::deinit(rtti::context& ctx) -> bool
{
    APPLOG_INFO("{}::{}", hpp::type_name_str(*this), __func__);

    return true;
}
} // namespace ace
