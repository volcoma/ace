#include "deferred_rendering.h"
#include <engine/assets/asset_manager.h>
#include <engine/ecs/components/camera_component.h>
#include <engine/ecs/components/light_component.h>
#include <engine/ecs/components/model_component.h>
#include <engine/ecs/components/reflection_probe_component.h>
#include <engine/ecs/components/transform_component.h>
#include <engine/ecs/systems/systems.h>

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
    auto rect = mesh.get()->calculate_screen_rect(world, cam);

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

auto should_rebuild_reflections(const visibility_set_models_t& visibility_set, const reflection_probe& probe) -> bool
{
    if(probe.method == reflect_method::environment)
        return true;

    for(const auto& element : visibility_set)
    {
        const auto& transform_comp_ref = element.get<transform_component>();
        const auto& model_comp_ref = element.get<model_component>();

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

        const auto& bounds = mesh->get_bounds();

        bool result = false;

        for(std::uint32_t i = 0; i < 6; ++i)
        {
            const auto& camera = camera::get_face_camera(i, world_transform);
            result |= camera.test_obb(bounds, world_transform);
        }

        if(result)
            return true;
    }

    return false;
}

auto should_rebuild_shadows(const visibility_set_models_t& visibility_set, const light&) -> bool
{
    for(const auto& element : visibility_set)
    {
        const auto& transform_comp_ref = element.get<transform_component>();
        const auto& model_comp_ref = element.get<model_component>();

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
        const auto& bounds = mesh->get_bounds();

        bool result = true;

        if(result)
            return true;
    }

    return false;
}

void deferred_rendering::on_frame_render(rtti::context& ctx, delta_t dt)
{
    auto& ec = ctx.get<ecs>();
    auto& scn = ec.get_scene();

    prepare_scene(scn, dt);
}

// void deferred_rendering::submit_material(gpu_program& program, const pbr_material* mat)
// {
//     const auto& color_map = mat->get_color_map();
//     const auto& normal_map = mat->get_normal_map();
//     const auto& roughness_map = mat->get_roughness_map();
//     const auto& metalness_map = mat->get_metalness_map();
//     const auto& ao_map = mat->get_ao_map();
//     const auto& emissive_map = mat->get_emissive_map();

//     const auto& albedo = color_map ? color_map : mat->default_color_map();
//     const auto& normal = normal_map ? normal_map : mat->default_normal_map();
//     const auto& roughness = roughness_map ? roughness_map : mat->default_color_map();
//     const auto& metalness = metalness_map ? metalness_map : mat->default_color_map();
//     const auto& ao = ao_map ? ao_map : mat->default_color_map();
//     const auto& emissive = emissive_map ? emissive_map : mat->default_color_map();

//     const auto& base_color = mat->get_base_color();
//     const auto& subsurface_color = mat->get_subsurface_color();
//     const auto& emissive_color = mat->get_emissive_color();
//     const auto& surface_data = mat->get_surface_data();
//     const auto& tiling = mat->get_tiling();
//     const auto& dither_threshold = mat->get_dither_threshold();
//     const auto& surface_data2 = mat->get_surface_data2();

//     program.set_texture(0, "s_tex_color", albedo.get().get());
//     program.set_texture(1, "s_tex_normal", normal.get().get());
//     program.set_texture(2, "s_tex_roughness", roughness.get().get());
//     program.set_texture(3, "s_tex_metalness", metalness.get().get());
//     program.set_texture(4, "s_tex_ao", ao.get().get());
//     program.set_texture(5, "s_tex_emissive", emissive.get().get());

//     program.set_uniform("u_base_color", base_color);
//     program.set_uniform("u_subsurface_color", subsurface_color);
//     program.set_uniform("u_emissive_color", emissive_color);
//     program.set_uniform("u_surface_data", surface_data);
//     program.set_uniform("u_tiling", tiling);
//     program.set_uniform("u_dither_threshold", dither_threshold);
//     program.set_uniform("u_surface_data2", surface_data2);
// }

void deferred_rendering::prepare_scene(scene& scn, delta_t dt)
{
    rendering_systems::on_frame_update(scn, dt);

    build_camera_independant_reflections(scn, dt);
    build_camera_independant_shadows(scn);
}

void deferred_rendering::build_camera_independant_reflections(scene& scn, delta_t dt)
{
    auto query = visibility_query::is_dirty | visibility_query::is_static | visibility_query::is_reflection_caster;

    auto dirty_models = gather_visible_models(scn, nullptr, query);
    scn.registry->view<transform_component, reflection_probe_component>().each(
        [&](auto e, auto&& transform_comp, auto&& reflection_probe_comp)
        {
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
                    visibility_set_models_t visibility_set;

                    if(probe.method != reflect_method::environment)
                    {
                        auto query = visibility_query::is_static | visibility_query::is_reflection_caster;
                        if(!should_rebuild)
                        {
                            query |= visibility_query::is_dirty;
                        }

                        visibility_set = gather_visible_models(scn, &camera, query);
                    }

                    gfx::frame_buffer::ptr output = nullptr;

                    build_per_camera_data(scn, camera, render_view, dt);
                    output = g_buffer_pass(output, visibility_set, camera, render_view, dt);
                    output = lighting_pass(output, scn, camera, render_view, dt);
                    output = atmospherics_pass(output, scn, camera, render_view, dt);
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

void deferred_rendering::build_camera_independant_shadows(scene& scn)
{
    build_shadows(scn, nullptr);
}

void deferred_rendering::build_camera_dependant_shadows(scene& scn, const camera& camera)
{
    build_shadows(scn, &camera);
}

void deferred_rendering::build_shadows(scene& scn, const camera* camera)
{
    auto query = visibility_query::is_dirty | visibility_query::is_shadow_caster;

    bool queried = false;
    visibility_set_models_t dirty_models;

    scn.registry->view<transform_component, light_component>().each(
        [&](auto e, auto&& transform_comp, auto&& light_comp)
        {
            // const auto& world_tranform = transform_comp.get_transform();
            const auto& light = light_comp.get_light();
            auto& generator = light_comp.get_shadowmap_generator();

            if(light.shadow_params.type == sm_impl::none)
            {
                return;
            }

            // directional light's require camera, as cascades are camera dependent
            if(light.type == light_type::directional && !camera)
            {
                return;
            }

            if(!queried)
            {
                dirty_models = gather_visible_models(scn, nullptr, query);
                queried = true;
            }


            bool should_rebuild = true;

            //            if(!transform_comp.is_touched() && !light_comp.is_touched())
            {
                // If shadows shouldn't be rebuilt - continue.
                should_rebuild = should_rebuild_shadows(dirty_models, light);
            }

            if(!should_rebuild)
                return;

            generator.generate_shadowmaps(light, transform_comp.get_transform_global(), dirty_models, camera);
        });
}

void deferred_rendering::build_per_camera_data(scene& scn,
                                               const camera& camera,
                                               gfx::render_view& render_view,
                                               delta_t dt)
{
    build_camera_dependant_shadows(scn, camera);
}

auto deferred_rendering::render_models(const visibility_set_models_t& visibility_set,
                                       scene& scn,
                                       const camera& camera,
                                       camera_storage& storage,
                                       gfx::render_view& render_view,
                                       delta_t dt) -> gfx::frame_buffer::ptr
{
    const auto& viewport_size = camera.get_viewport_size();
    auto target = render_view.get_output_fbo(viewport_size);

    render_models(target, visibility_set, scn, camera, storage, render_view, dt);

    return target;
}

void deferred_rendering::render_models(const std::shared_ptr<gfx::frame_buffer>& output,
                                       const visibility_set_models_t& visibility_set,
                                       scene& scn,
                                       const camera& camera,
                                       camera_storage& storage,
                                       gfx::render_view& render_view,
                                       delta_t dt)
{
    gfx::frame_buffer::ptr target = nullptr;

    build_per_camera_data(scn, camera, render_view, dt);

    target = g_buffer_pass(target, visibility_set, camera, render_view, dt);

    target = reflection_probe_pass(target, scn, camera, render_view, dt);

    target = lighting_pass(target, scn, camera, render_view, dt);

    target = atmospherics_pass(target, scn, camera, render_view, dt);

    tonemapping_pass(target, output);
}

auto deferred_rendering::g_buffer_pass(gfx::frame_buffer::ptr input,
                                       const visibility_set_models_t& visibility_set,
                                       const camera& camera,
                                       gfx::render_view& render_view,
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

    for(const auto& e : visibility_set)
    {
        const auto& transform_comp = e.get<transform_component>();
        const auto& model_comp = e.get<model_component>();

        const auto& model = model_comp.get_model();
        if(!model.is_valid())
            continue;

        const auto& world_transform = transform_comp.get_transform_global();
        const auto clip_planes = math::vec2(camera.get_near_clip(), camera.get_far_clip());

        lod_data lod_runtime_data{}; // camera_lods[e];
        const auto transition_time = 0.0f;
        const auto lod_count = model.get_lods().size();
        const auto& lod_limits = model.get_lod_limits();

        const auto base_mesh = model.get_lod(0);
        if(!base_mesh)
            continue;

        if(false == update_lod_data(lod_runtime_data,
                                    lod_limits,
                                    lod_count,
                                    transition_time,
                                    dt.count(),
                                    base_mesh,
                                    world_transform,
                                    camera))
            continue;

        const auto current_time = lod_runtime_data.current_time;
        const auto current_lod_index = lod_runtime_data.current_lod_index;
        const auto target_lod_index = lod_runtime_data.target_lod_index;

        const auto params = math::vec3{0.0f, -1.0f, (transition_time - current_time) / transition_time};

        const auto params_inv = math::vec3{1.0f, 1.0f, current_time / transition_time};

        const auto& bone_transforms = model_comp.get_bone_transforms();

        model.submit(pass.id,
                     world_transform,
                     bone_transforms,
                     true,
                     true,
                     true,
                     current_lod_index,
                     geom_program_.get(),
                     geom_skinned_program_.get(),
                     [&](auto& p)
                     {
                         auto camera_pos = camera.get_position();
                         p.set_uniform("u_camera_wpos", camera_pos);
                         p.set_uniform("u_camera_clip_planes", clip_planes);
                         p.set_uniform("u_lod_params", params);
                     });

        if(math::epsilonNotEqual(current_time, 0.0f, math::epsilon<float>()))
        {
            model.submit(pass.id,
                         world_transform,
                         bone_transforms,
                         true,
                         true,
                         true,
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
                                       scene& scn,
                                       const camera& camera,
                                       gfx::render_view& render_view,
                                       delta_t dt) -> gfx::frame_buffer::ptr
{
    const auto& view = camera.get_view();
    const auto& proj = camera.get_projection();
    const auto& camera_pos = camera.get_position();

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

    scn.registry->view<transform_component, light_component>().each(
        [&](auto e, auto&& transform_comp_ref, auto&& light_comp_ref)
        {
            const auto& light = light_comp_ref.get_light();
            const auto& generator = light_comp_ref.get_shadowmap_generator();
            const auto& world_transform = transform_comp_ref.get_transform_global();
            const auto& light_position = world_transform.get_position();
            const auto& light_direction = world_transform.z_unit_axis();

            irect32_t rect(0, 0, irect32_t::value_type(buffer_size.width), irect32_t::value_type(buffer_size.height));
            if(light_comp_ref
                   .compute_projected_sphere_rect(rect, light_position, light_direction, camera_pos, view, proj) == 0)
                return;

            auto program = generator.get_color_apply_program(light);
            if(!program)
            {
                return;
            }

            program->begin();

            // gpu_program* program = nullptr;
            if(light.type == light_type::directional)
            {
                // Draw light.
                program->set_uniform("u_light_direction", light_direction);
            }
            if(light.type == light_type::point)
            {
                float light_data[4] = {light.point_data.range, light.point_data.exponent_falloff, 0.0f, 0.0f};

                // Draw light.
                program->set_uniform("u_light_position", light_position);
                program->set_uniform("u_light_data", light_data);
            }

            if(light.type == light_type::spot)
            {
                float light_data[4] = {light.spot_data.get_range(),
                                       math::cos(math::radians(light.spot_data.get_inner_angle() * 0.5f)),
                                       math::cos(math::radians(light.spot_data.get_outer_angle() * 0.5f)),
                                       0.0f};

                // Draw light.
                program->set_uniform("u_light_position", light_position);
                program->set_uniform("u_light_direction", light_direction);
                program->set_uniform("u_light_data", light_data);
            }

            float light_color_intensity[4] = {light.color.value.r,
                                              light.color.value.g,
                                              light.color.value.b,
                                              light.intensity};
            program->set_uniform("u_light_color_intensity", light_color_intensity);
            program->set_uniform("u_camera_position", camera_pos);
            program->set_texture(0, "s_tex0", g_buffer_fbo->get_texture(0).get());
            program->set_texture(1, "s_tex1", g_buffer_fbo->get_texture(1).get());
            program->set_texture(2, "s_tex2", g_buffer_fbo->get_texture(2).get());
            program->set_texture(3, "s_tex3", g_buffer_fbo->get_texture(3).get());
            program->set_texture(4, "s_tex4", g_buffer_fbo->get_texture(4).get());
            program->set_texture(5, "s_tex5", refl_buffer);
            program->set_texture(6, "s_tex6", ibl_brdf_lut_.get().get());

            if(light.shadow_params.type != sm_impl::none)
            {
                generator.submit_uniforms();
            }
            gfx::set_scissor(rect.left, rect.top, rect.width(), rect.height());
            auto topology = gfx::clip_quad(1.0f);
            gfx::set_state(topology | BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_BLEND_ADD);
            gfx::submit(pass.id, program->native_handle());
            gfx::set_state(BGFX_STATE_DEFAULT);

            program->end();
        });

    gfx::discard();

    return l_buffer_fbo;
}

auto deferred_rendering::reflection_probe_pass(gfx::frame_buffer::ptr input,
                                               scene& scn,
                                               const camera& camera,
                                               gfx::render_view& render_view,
                                               delta_t dt) -> gfx::frame_buffer::ptr
{
    const auto& view = camera.get_view();
    const auto& proj = camera.get_projection();
    const auto& camera_pos = camera.get_position();

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

    scn.registry->view<transform_component, reflection_probe_component>().each(
        [&](auto e, auto&& transform_comp_ref, auto&& probe_comp_ref)
        {
            const auto& probe = probe_comp_ref.get_probe();
            const auto& world_transform = transform_comp_ref.get_transform_global();
            const auto& probe_position = world_transform.get_position();

            irect32_t rect(0, 0, irect32_t::value_type(buffer_size.width), irect32_t::value_type(buffer_size.height));
            if(probe_comp_ref.compute_projected_sphere_rect(rect, probe_position, camera_pos, view, proj) == 0)
                return;

            const auto cubemap = probe_comp_ref.get_cubemap();

            ref_probe_program* ref_probe_program = nullptr;
            float influence_radius = 0.0f;
            if(probe.type == probe_type::sphere && sphere_ref_probe_program_.program)
            {
                ref_probe_program = &sphere_ref_probe_program_;
                influence_radius = probe.sphere_data.range;
            }

            if(probe.type == probe_type::box && box_ref_probe_program_.program)
            {
                math::transform t;
                t.set_scale(probe.box_data.extents);
                t = world_transform * t;
                auto u_inv_world = math::inverse(t).get_matrix();
                float data2[4] = {probe.box_data.extents.x,
                                  probe.box_data.extents.y,
                                  probe.box_data.extents.z,
                                  probe.box_data.transition_distance};

                ref_probe_program = &box_ref_probe_program_;

                gfx::set_uniform(box_ref_probe_program_.u_inv_world, u_inv_world);
                gfx::set_uniform(box_ref_probe_program_.u_data2, data2);


                influence_radius = math::length(t.get_scale() + probe.box_data.transition_distance);
            }

            if(ref_probe_program)
            {
                float mips = cubemap ? float(cubemap->info.numMips) : 1.0f;
                float data0[4] = {
                    probe_position.x,
                    probe_position.y,
                    probe_position.z,
                    influence_radius,
                };

                float data1[4] = {mips, 0.0f, 0.0f, 0.0f};


                gfx::set_uniform(ref_probe_program->u_data0, data0);
                gfx::set_uniform(ref_probe_program->u_data1, data1);

                for(size_t i = 0; i < 5; ++i)
                {
                    gfx::set_texture(ref_probe_program->s_tex[i], i, g_buffer_fbo->get_texture(i).get());
                }

                gfx::set_texture(ref_probe_program->s_tex_cube, 5, cubemap.get());

                gfx::set_scissor(rect.left, rect.top, rect.width(), rect.height());
                auto topology = gfx::clip_quad(1.0f);
                gfx::set_state(topology | BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_BLEND_ALPHA);

                ref_probe_program->program->begin();
                gfx::submit(pass.id, ref_probe_program->program->native_handle());
                gfx::set_state(BGFX_STATE_DEFAULT);
                ref_probe_program->program->end();
            }
        });

    gfx::discard();

    return r_buffer_fbo;
}

auto deferred_rendering::atmospherics_pass(gfx::frame_buffer::ptr input,
                                           scene& scn,
                                           const camera& camera,
                                           gfx::render_view& render_view,
                                           delta_t dt) -> gfx::frame_buffer::ptr
{
    atmospheric_pass::run_params params;
    atmospheric_pass_perez::run_params params_perez;

    bool found_sun = false;

    skylight_component::sky_mode mode{};
    scn.registry->view<transform_component, skylight_component>().each(
        [&](auto e, auto&& transform_comp_ref, auto&& light_comp_ref)
        {
            auto entity = scn.create_entity(e);

            if(found_sun)
            {
                APPLOG_WARNING("[{}] More than one entity with this component. Others are ignored.", "Skylight");
                return;
            }

            mode = light_comp_ref.get_mode();
            found_sun = true;
            if(auto light_comp = entity.template try_get<light_component>())
            {
                const auto& light = light_comp->get_light();

                if(light.type == light_type::directional)
                {
                    const auto& world_transform = transform_comp_ref.get_transform_global();
                    params.light_direction = world_transform.z_unit_axis();

                    params_perez.light_direction = world_transform.z_unit_axis();
                }
            }
        });

    if(!found_sun)
    {
        return input;
    }
    const auto& viewport_size = camera.get_viewport_size();

    auto c = camera;
    c.set_projection_mode(projection_mode::perspective);
    input = render_view.get_fbo("LBUFFER", {input->get_texture(0), render_view.get_depth_buffer(viewport_size)});

    switch(mode)
    {
        case skylight_component::sky_mode::perez:
            return atmospheric_pass_perez_.run(input, c, dt, params_perez);

        default:
            return atmospheric_pass_.run(input, c, dt, params);
    }
}

auto deferred_rendering::tonemapping_pass(gfx::frame_buffer::ptr input,
                                          const camera& camera,
                                          gfx::render_view& render_view) -> gfx::frame_buffer::ptr
{
    if(!input)
        return nullptr;

    const auto& viewport_size = camera.get_viewport_size();
    auto surface = render_view.get_output_fbo(viewport_size);

    tonemapping_pass(input, surface);

    return surface;
}

void deferred_rendering::tonemapping_pass(gfx::frame_buffer::ptr input, std::shared_ptr<gfx::frame_buffer> output)
{
    if(!input)
        return;

    const auto output_size = output->get_size();
    gfx::render_pass pass("output_buffer_fill");
    pass.bind(output.get());

    gamma_correction_program_.program->begin();
    gfx::set_texture(gamma_correction_program_.s_input, 0, input->get_texture().get());
    irect32_t rect(0, 0, irect32_t::value_type(output_size.width), irect32_t::value_type(output_size.height));
    gfx::set_scissor(rect.left, rect.top, rect.width(), rect.height());
    auto topology = gfx::clip_quad(1.0f);
    gfx::set_state(topology | BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A);
    gfx::submit(pass.id, gamma_correction_program_.program->native_handle());
    gfx::set_state(BGFX_STATE_DEFAULT);
    gamma_correction_program_.program->end();


    gfx::discard();
}

deferred_rendering::deferred_rendering()
{
}

deferred_rendering::~deferred_rendering()
{
}

void benchmark_test_obb(const math::frustum& f, const math::bbox& AABB, const math::transform& t)
{
    const int iterations = 100000;

    {
        // Original approach benchmark
        auto start = std::chrono::high_resolution_clock::now();
        for(int i = 0; i < iterations; ++i)
        {
            bool result = f.test_obb(AABB, t);
        }
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> original_duration = end - start;
        std::cout << "Original approach duration: " << original_duration.count() << " seconds" << std::endl;
    }

    {
        // Optimized approach benchmark
        auto start = std::chrono::high_resolution_clock::now();
        for(int i = 0; i < iterations; ++i)
        {
            bool result = f.test_obb(AABB, t);
        }
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> optimized_duration = end - start;
        std::cout << "Optimized1 approach duration: " << optimized_duration.count() << " seconds" << std::endl;
    }

    {
        // Optimized approach benchmark
        auto start = std::chrono::high_resolution_clock::now();
        for(int i = 0; i < iterations; ++i)
        {
            bool result = f.test_obb(AABB, t);
        }
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> optimized_duration = end - start;
        std::cout << "Optimized2 approach duration: " << optimized_duration.count() << " seconds" << std::endl;
    }
}

auto deferred_rendering::init(rtti::context& ctx) -> bool
{
    APPLOG_INFO("{}::{}", hpp::type_name_str(*this), __func__);

    // Setup your frustum, bbox, and transform objects here
    math::frustum f;
    math::bbox AABB;
    math::transform t;

    // Initialize the frustum, bbox, and transform with appropriate values

    benchmark_test_obb(f, AABB, t);

    auto& ev = ctx.get<events>();
    ev.on_frame_render.connect(sentinel_, 1000, this, &deferred_rendering::on_frame_render);

    auto& am = ctx.get<asset_manager>();

    auto loadProgram = [&](const std::string& vs, const std::string& fs)
    {
        auto vs_shader = am.get_asset<gfx::shader>("engine:/data/shaders/" + vs + ".sc");
        auto fs_shadfer = am.get_asset<gfx::shader>("engine:/data/shaders/" + fs + ".sc");

        return std::make_unique<gpu_program>(vs_shader, fs_shadfer);
    };

    geom_program_ = loadProgram("vs_deferred_geom", "fs_deferred_geom");
    geom_skinned_program_ = loadProgram("vs_deferred_geom_skinned", "fs_deferred_geom");
    gamma_correction_program_.program = loadProgram("vs_clip_quad", "fs_gamma_correction");
    gamma_correction_program_.cache_uniforms();

    sphere_ref_probe_program_.program = loadProgram("vs_clip_quad_ex", "fs_sphere_reflection_probe");
    sphere_ref_probe_program_.cache_uniforms();

    box_ref_probe_program_.program = loadProgram("vs_clip_quad_ex", "fs_box_reflection_probe");
    box_ref_probe_program_.cache_uniforms();

    geom_program_ = loadProgram("vs_deferred_geom", "fs_deferred_geom");

    ibl_brdf_lut_ = am.get_asset<gfx::texture>("engine:/data/textures/ibl_brdf_lut.png");

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
