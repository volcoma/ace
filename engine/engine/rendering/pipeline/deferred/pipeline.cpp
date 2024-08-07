#include "pipeline.h"
#include <engine/assets/asset_manager.h>
#include <engine/ecs/components/camera_component.h>
#include <engine/ecs/components/light_component.h>
#include <engine/ecs/components/model_component.h>
#include <engine/ecs/components/reflection_probe_component.h>
#include <engine/ecs/components/transform_component.h>

#include <engine/engine.h>
#include <engine/rendering/camera.h>
#include <engine/rendering/material.h>
#include <engine/rendering/mesh.h>
#include <engine/rendering/model.h>
#include <engine/rendering/renderer.h>

#include <engine/profiler/profiler.h>

#include <graphics/index_buffer.h>
#include <graphics/render_pass.h>
#include <graphics/render_view.h>
#include <graphics/texture.h>
#include <graphics/vertex_buffer.h>

namespace ace
{
namespace rendering
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

auto should_rebuild_shadows(const visibility_set_models_t& visibility_set,
                            const light& light,
                            const math::bbox& light_bounds,
                            const math::transform& light_transform) -> bool
{
    auto lbounds = math::bbox::mul(light_bounds, light_transform);
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

        auto mbounds = math::bbox::mul(bounds, world_transform);

        bool result = lbounds.intersect(mbounds);

        if(result)
            return true;
    }

    return false;
}

auto deferred::get_light_program(const light& l) const -> const color_lighting&
{
    return color_lighting_[uint8_t(l.type)][uint8_t(l.shadow_params.depth)][uint8_t(l.shadow_params.type)];
}

auto deferred::get_light_program_no_shadows(const light& l) const -> const color_lighting&
{
    return color_lighting_no_shadow_[uint8_t(l.type)];
}

void deferred::submit_material(geom_program& program, const pbr_material& mat)
{
    const auto& color_map = mat.get_color_map();
    const auto& normal_map = mat.get_normal_map();
    const auto& roughness_map = mat.get_roughness_map();
    const auto& metalness_map = mat.get_metalness_map();
    const auto& ao_map = mat.get_ao_map();
    const auto& emissive_map = mat.get_emissive_map();

    const auto& albedo = color_map ? color_map : mat.default_color_map();
    const auto& normal = normal_map ? normal_map : mat.default_normal_map();
    const auto& roughness = roughness_map ? roughness_map : mat.default_color_map();
    const auto& metalness = metalness_map ? metalness_map : mat.default_color_map();
    const auto& ao = ao_map ? ao_map : mat.default_color_map();
    const auto& emissive = emissive_map ? emissive_map : mat.default_color_map();

    const auto& base_color = mat.get_base_color();
    const auto& subsurface_color = mat.get_subsurface_color();
    const auto& emissive_color = mat.get_emissive_color();
    const auto& surface_data = mat.get_surface_data();
    const auto& tiling = mat.get_tiling();
    const auto& dither_threshold = mat.get_dither_threshold();
    const auto& surface_data2 = mat.get_surface_data2();

    gfx::set_texture(program.s_tex_color, 0, albedo.get());
    gfx::set_texture(program.s_tex_normal, 1, normal.get());
    gfx::set_texture(program.s_tex_roughness, 2, roughness.get());
    gfx::set_texture(program.s_tex_metalness, 3, metalness.get());
    gfx::set_texture(program.s_tex_ao, 4, ao.get());
    gfx::set_texture(program.s_tex_emissive, 5, emissive.get());

    gfx::set_uniform(program.u_base_color, base_color);
    gfx::set_uniform(program.u_subsurface_color, subsurface_color);
    gfx::set_uniform(program.u_emissive_color, emissive_color);
    gfx::set_uniform(program.u_surface_data, surface_data);
    gfx::set_uniform(program.u_tiling, tiling);
    gfx::set_uniform(program.u_dither_threshold, dither_threshold);
    gfx::set_uniform(program.u_surface_data2, surface_data2);

    auto state = mat.get_render_states(true, true, true);

    gfx::set_state(state);
}

void deferred::build_reflections(scene& scn, const camera& camera, delta_t dt)
{
    APP_SCOPE_PERF("Reflection Generation Pass");

    auto query = visibility_query::is_dirty | visibility_query::is_static | visibility_query::is_reflection_caster;

    auto dirty_models = gather_visible_models(scn, nullptr, query);
    scn.registry->view<transform_component, reflection_probe_component>().each(
        [&](auto e, auto&& transform_comp, auto&& reflection_probe_comp)
        {
            if(reflection_probe_comp.already_generated())
            {
                return;
            }

            // reflection_probe_comp.set_generation_frame(gfx::get_render_frame());

            const auto& world_transform = transform_comp.get_transform_global();

            const auto& bounds = reflection_probe_comp.get_bounds();
            if(!camera.test_obb(bounds, world_transform))
            {
                return;
            }

            const auto& probe = reflection_probe_comp.get_probe();

            auto cubemap_fbo = reflection_probe_comp.get_cubemap_fbo();
            bool should_rebuild = should_rebuild_reflections(dirty_models, probe);

            // If reflections shouldn't be rebuilt - continue.
            if(!should_rebuild)
                return;

            {
                // iterate trough each cube face
                for(std::uint32_t face = 0; face < 6; ++face)
                {
                    if(reflection_probe_comp.already_generated(face))
                    {
                        continue;
                    }

                    reflection_probe_comp.set_generation_frame(face, gfx::get_render_frame());

                    auto camera = camera::get_face_camera(face, world_transform);
                    camera.set_far_clip(reflection_probe_comp.get_probe().box_data.extents.r);
                    auto& render_view = reflection_probe_comp.get_render_view(face);
                    camera.set_viewport_size(usize32_t(cubemap_fbo->get_size()));
                    visibility_set_models_t visibility_set;
                    gfx::frame_buffer::ptr output = nullptr;

                    bool not_environment = probe.method != reflect_method::environment;

                    pipeline_flags flags = pipeline_steps::probe;
                    visibility_flags vis_flags = visibility_query::is_reflection_caster;

                    if(not_environment)
                    {
                        flags |= pipeline_steps::shadow_pass;
                        flags |= pipeline_steps::geometry_pass;
                    }

                    gfx::render_pass::push_scope("build.reflecitons");
                    output = run_pipeline_impl(flags, scn, camera, render_view, dt, vis_flags);
                    gfx::render_pass::pop_scope();

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

void deferred::build_shadows(scene& scn, const camera& camera, visibility_flags query)
{
    APP_SCOPE_PERF("Shadow Generation Pass");

    query |= visibility_query::is_dirty | visibility_query::is_shadow_caster;

    bool queried = false;
    visibility_set_models_t dirty_models;

    const auto& view = camera.get_view();
    const auto& proj = camera.get_projection();
    const auto& camera_pos = camera.get_position();

    scn.registry->view<transform_component, light_component>().each(
        [&](auto e, auto&& transform_comp, auto&& light_comp)
        {
            const auto& light = light_comp.get_light();

            bool camera_dependant = light.type == light_type::directional;

            auto& generator = light_comp.get_shadowmap_generator();
            if(!camera_dependant && generator.already_updated())
            {
                return;
            }

            APP_SCOPE_PERF("Shadow Generation Pass Per Light");

            const auto& world_transform = transform_comp.get_transform_global();
            const auto& light_direction = world_transform.z_unit_axis();

            const auto& bounds = light_comp.get_bounds_precise(light_direction);
            if(!camera.test_obb(bounds, world_transform))
            {
                return;
            }

            generator.update(camera, light, world_transform);

            if(!light.casts_shadows)
            {
                return;
            }

            if(!queried)
            {
                dirty_models = gather_visible_models(scn, nullptr, query);
                queried = true;
            }

            bool should_rebuild = should_rebuild_shadows(dirty_models, light, bounds, world_transform);

            // If shadows shouldn't be rebuilt - continue.
            if(!should_rebuild)
                return;

            APP_SCOPE_PERF("Shadow Generation Pass Per Light After Cull");

            generator.generate_shadowmaps(dirty_models);
        });
}

auto deferred::run_pipeline(scene& scn,
                            const camera& camera,
                            camera_storage& storage,
                            gfx::render_view& render_view,
                            delta_t dt,
                            visibility_flags query) -> gfx::frame_buffer::ptr
{
    const auto& viewport_size = camera.get_viewport_size();
    auto target = render_view.get_output_fbo(viewport_size);

    run_pipeline(target, scn, camera, storage, render_view, dt, query);

    return target;
}

void deferred::run_pipeline(const gfx::frame_buffer::ptr& output,
                            scene& scn,
                            const camera& camera,
                            camera_storage& storage,
                            gfx::render_view& render_view,
                            delta_t dt,
                            visibility_flags query)
{
    pipeline_flags pipeline = pipeline_steps::full;
    run_pipeline_impl(pipeline, output, scn, camera, render_view, dt, query);
}

auto deferred::run_pipeline_impl(pipeline_flags pipeline,
                                 scene& scn,
                                 const camera& camera,
                                 gfx::render_view& render_view,
                                 delta_t dt,
                                 visibility_flags query) -> gfx::frame_buffer::ptr
{
    const auto& viewport_size = camera.get_viewport_size();
    auto target = render_view.get_output_fbo(viewport_size);

    run_pipeline_impl(pipeline, target, scn, camera, render_view, dt, query);

    return target;
}

void deferred::run_pipeline_impl(pipeline_flags pipeline,
                                 const gfx::frame_buffer::ptr& output,
                                 scene& scn,
                                 const camera& camera,
                                 gfx::render_view& render_view,
                                 delta_t dt,
                                 visibility_flags query)
{
    APP_SCOPE_PERF("Full Pass");

    visibility_set_models_t visibility_set;
    gfx::frame_buffer::ptr target = nullptr;

    bool apply_reflecitons = pipeline & pipeline_steps::reflection_probe;
    bool apply_shadows = pipeline & pipeline_steps::shadow_pass;
    if(apply_reflecitons)
    {
        build_reflections(scn, camera, dt);
    }

    if(apply_shadows)
    {
        build_shadows(scn, camera, query);
    }

    if(pipeline & pipeline_steps::geometry_pass)
    {
        visibility_set = gather_visible_models(scn, &camera.get_frustum(), query);
    }
    target = run_g_buffer_pass(target, visibility_set, camera, render_view, dt);

    if(pipeline & pipeline_steps::assao)
    {
        run_assao_pass(target, visibility_set, camera, render_view, dt);
    }

    if(apply_reflecitons)
    {
        target = run_reflection_probe_pass(target, scn, camera, render_view, dt);
    }

    target = run_lighting_pass(target, scn, camera, render_view, apply_shadows, dt);

    target = run_atmospherics_pass(target, scn, camera, render_view, dt);

    run_tonemapping_pass(target, output);
}

auto deferred::run_g_buffer_pass(gfx::frame_buffer::ptr input,
                                 const visibility_set_models_t& visibility_set,
                                 const camera& camera,
                                 gfx::render_view& render_view,
                                 delta_t dt) -> gfx::frame_buffer::ptr
{
    APP_SCOPE_PERF("G-Buffer Pass");

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

        auto camera_pos = camera.get_position();

        model::submit_callbacks callbacks;
        callbacks.setup_begin = [&](const model::submit_callbacks::params& submit_params)
        {
            geom_program& prog = submit_params.skinned ? geom_program_skinned_ : geom_program_;

            prog.program->begin();

            gfx::set_uniform(prog.u_camera_wpos, camera_pos);
            gfx::set_uniform(prog.u_camera_clip_planes, clip_planes);
        };
        callbacks.setup_params_per_instance = [&](const model::submit_callbacks::params& submit_params)
        {
            geom_program& prog = submit_params.skinned ? geom_program_skinned_ : geom_program_;

            gfx::set_uniform(prog.u_lod_params, params);
        };
        callbacks.setup_params_per_subset =
            [&](const model::submit_callbacks::params& submit_params, const material& mat)
        {
            geom_program& prog = submit_params.skinned ? geom_program_skinned_ : geom_program_;

            if(rttr::type::get(mat) == rttr::type::get<pbr_material>())
            {
                const auto& pbr = static_cast<const pbr_material&>(mat);
                submit_material(prog, pbr);
            }
            else
            {
                mat.submit(prog.program.get());
            }

            gfx::submit(pass.id, prog.program->native_handle());
        };
        callbacks.setup_end = [&](const model::submit_callbacks::params& submit_params)
        {
            geom_program& prog = submit_params.skinned ? geom_program_skinned_ : geom_program_;

            prog.program->end();
        };

        model.submit(world_transform, bone_transforms, current_lod_index, callbacks);
        if(math::epsilonNotEqual(current_time, 0.0f, math::epsilon<float>()))
        {
            callbacks.setup_params_per_instance = [&](const model::submit_callbacks::params& submit_params)
            {
                geom_program& prog = submit_params.skinned ? geom_program_skinned_ : geom_program_;

                gfx::set_uniform(prog.u_lod_params, params);
            };

            model.submit(world_transform, bone_transforms, target_lod_index, callbacks);
        }
    }
    gfx::discard();

    return g_buffer_fbo;
}

auto deferred::run_assao_pass(gfx::frame_buffer::ptr input,
                              const visibility_set_models_t& visibility_set,
                              const camera& camera,
                              gfx::render_view& render_view,
                              delta_t dt) -> gfx::frame_buffer::ptr
{
    APP_SCOPE_PERF("Assao Pass");
    const auto& viewport_size = camera.get_viewport_size();
    auto g_buffer_fbo = render_view.get_g_buffer_fbo(viewport_size);
    auto color_ao = g_buffer_fbo->get_texture(0);
    auto normal = g_buffer_fbo->get_texture(1);
    auto depth = g_buffer_fbo->get_texture(4);

    assao_pass::run_params params;
    params.depth = depth.get();
    params.normal = normal.get();
    params.color_ao = color_ao.get();

    assao_pass_.run(camera, params);

    return input;
}

auto deferred::run_lighting_pass(gfx::frame_buffer::ptr input,
                                 scene& scn,
                                 const camera& camera,
                                 gfx::render_view& render_view,
                                 bool apply_shadows,
                                 delta_t dt) -> gfx::frame_buffer::ptr
{
    APP_SCOPE_PERF("Lighting Pass");

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
        render_view.get_texture("RBUFFER", viewport_size.width, viewport_size.height, false, 1, light_buffer_format);

    scn.registry->view<transform_component, light_component>().each(
        [&](auto e, auto&& transform_comp_ref, auto&& light_comp_ref)
        {
            const auto& light = light_comp_ref.get_light();
            const auto& generator = light_comp_ref.get_shadowmap_generator();
            const auto& world_transform = transform_comp_ref.get_transform_global();
            const auto& light_position = world_transform.get_position();
            const auto& light_direction = world_transform.z_unit_axis();

            const auto& bounds = light_comp_ref.get_bounds_precise(light_direction);
            if(!camera.test_obb(bounds, world_transform))
            {
                return;
            }

            irect32_t rect(0, 0, irect32_t::value_type(buffer_size.width), irect32_t::value_type(buffer_size.height));
            if(light_comp_ref
                   .compute_projected_sphere_rect(rect, light_position, light_direction, camera_pos, view, proj) == 0)
                return;

            APP_SCOPE_PERF("Lighting Pass Per Light");

            bool has_shadows = light.casts_shadows && apply_shadows;

            const auto& lprogram = has_shadows ? get_light_program(light) : get_light_program_no_shadows(light);

            lprogram.program->begin();

            if(light.type == light_type::directional)
            {
                gfx::set_uniform(lprogram.u_light_direction, light_direction);
            }
            if(light.type == light_type::point)
            {
                float light_data[4] = {light.point_data.range, light.point_data.exponent_falloff, 0.0f, 0.0f};

                gfx::set_uniform(lprogram.u_light_position, light_position);
                gfx::set_uniform(lprogram.u_light_data, light_data);
            }

            if(light.type == light_type::spot)
            {
                float light_data[4] = {light.spot_data.get_range(),
                                       math::cos(math::radians(light.spot_data.get_inner_angle() * 0.5f)),
                                       math::cos(math::radians(light.spot_data.get_outer_angle() * 0.5f)),
                                       0.0f};

                gfx::set_uniform(lprogram.u_light_direction, light_direction);
                gfx::set_uniform(lprogram.u_light_position, light_position);
                gfx::set_uniform(lprogram.u_light_data, light_data);
            }

            float light_color_intensity[4] = {light.color.value.r,
                                              light.color.value.g,
                                              light.color.value.b,
                                              light.intensity};

            gfx::set_uniform(lprogram.u_light_color_intensity, light_color_intensity);
            gfx::set_uniform(lprogram.u_camera_position, camera_pos);
            gfx::set_texture(lprogram.s_tex0, 0, g_buffer_fbo->get_texture(0));
            gfx::set_texture(lprogram.s_tex1, 1, g_buffer_fbo->get_texture(1));
            gfx::set_texture(lprogram.s_tex2, 2, g_buffer_fbo->get_texture(2));
            gfx::set_texture(lprogram.s_tex3, 3, g_buffer_fbo->get_texture(3));
            gfx::set_texture(lprogram.s_tex4, 4, g_buffer_fbo->get_texture(4));
            gfx::set_texture(lprogram.s_tex5, 5, refl_buffer);
            gfx::set_texture(lprogram.s_tex6, 6, ibl_brdf_lut_.get());

            if(has_shadows)
            {
                generator.submit_uniforms(7);
            }
            gfx::set_scissor(rect.left, rect.top, rect.width(), rect.height());
            auto topology = gfx::clip_quad(1.0f);
            gfx::set_state(topology | BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_BLEND_ADD);
            gfx::submit(pass.id, lprogram.program->native_handle());
            gfx::set_state(BGFX_STATE_DEFAULT);

            lprogram.program->end();
        });

    gfx::discard();

    return l_buffer_fbo;
}

auto deferred::run_reflection_probe_pass(gfx::frame_buffer::ptr input,
                                         scene& scn,
                                         const camera& camera,
                                         gfx::render_view& render_view,
                                         delta_t dt) -> gfx::frame_buffer::ptr
{
    APP_SCOPE_PERF("Reflection Probe Pass");

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
                    gfx::set_texture(ref_probe_program->s_tex[i], i, g_buffer_fbo->get_texture(i));
                }

                gfx::set_texture(ref_probe_program->s_tex_cube, 5, cubemap);

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

auto deferred::run_atmospherics_pass(gfx::frame_buffer::ptr input,
                                     scene& scn,
                                     const camera& camera,
                                     gfx::render_view& render_view,
                                     delta_t dt) -> gfx::frame_buffer::ptr
{
    APP_SCOPE_PERF("Atmospheric Pass");

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
                    params.turbidity = light_comp_ref.get_turbidity();

                    params_perez.light_direction = world_transform.z_unit_axis();
                    params_perez.turbidity = light_comp_ref.get_turbidity();
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

auto deferred::run_tonemapping_pass(gfx::frame_buffer::ptr input, const camera& camera, gfx::render_view& render_view)
    -> gfx::frame_buffer::ptr
{
    if(!input)
        return nullptr;

    const auto& viewport_size = camera.get_viewport_size();
    auto surface = render_view.get_output_fbo(viewport_size);

    run_tonemapping_pass(input, surface);

    return surface;
}

void deferred::run_tonemapping_pass(gfx::frame_buffer::ptr input, gfx::frame_buffer::ptr output)
{
    if(!input)
        return;

    APP_SCOPE_PERF("Tonemapping Pass");

    tonemapping_pass_.run(input, output);
}

deferred::deferred()
{
    init(engine::context());
}

deferred::~deferred()
{
    deinit(engine::context());
}

auto deferred::init(rtti::context& ctx) -> bool
{
    auto& am = ctx.get<asset_manager>();

    auto loadProgram = [&](const std::string& vs, const std::string& fs)
    {
        auto vs_shader = am.get_asset<gfx::shader>("engine:/data/shaders/" + vs + ".sc");
        auto fs_shadfer = am.get_asset<gfx::shader>("engine:/data/shaders/" + fs + ".sc");

        return std::make_unique<gpu_program>(vs_shader, fs_shadfer);
    };

    geom_program_.program = loadProgram("vs_deferred_geom", "fs_deferred_geom");
    geom_program_.cache_uniforms();

    geom_program_skinned_.program = loadProgram("vs_deferred_geom_skinned", "fs_deferred_geom");
    geom_program_skinned_.cache_uniforms();

    sphere_ref_probe_program_.program = loadProgram("vs_clip_quad_ex", "fs_sphere_reflection_probe");
    sphere_ref_probe_program_.cache_uniforms();

    box_ref_probe_program_.program = loadProgram("vs_clip_quad_ex", "fs_box_reflection_probe");
    box_ref_probe_program_.cache_uniforms();

    // Color lighting.

    // clang-format off
    color_lighting_no_shadow_[uint8_t(light_type::spot)].program = loadProgram("vs_clip_quad", "fs_deferred_spot_light");
    color_lighting_[uint8_t(light_type::spot)][uint8_t(sm_depth::invz)][uint8_t(sm_impl::hard)].program = loadProgram("vs_clip_quad", "fs_deferred_spot_light_hard");
    color_lighting_[uint8_t(light_type::spot)][uint8_t(sm_depth::invz)][uint8_t(sm_impl::pcf) ].program = loadProgram("vs_clip_quad", "fs_deferred_spot_light_pcf");
    color_lighting_[uint8_t(light_type::spot)][uint8_t(sm_depth::invz)][uint8_t(sm_impl::vsm) ].program = loadProgram("vs_clip_quad", "fs_deferred_spot_light_vsm");
    color_lighting_[uint8_t(light_type::spot)][uint8_t(sm_depth::invz)][uint8_t(sm_impl::esm) ].program = loadProgram("vs_clip_quad", "fs_deferred_spot_light_esm");

    color_lighting_[uint8_t(light_type::spot)][uint8_t(sm_depth::linear)][uint8_t(sm_impl::hard)].program = loadProgram("vs_clip_quad", "fs_deferred_spot_light_hard_linear");
    color_lighting_[uint8_t(light_type::spot)][uint8_t(sm_depth::linear)][uint8_t(sm_impl::pcf) ].program = loadProgram("vs_clip_quad", "fs_deferred_spot_light_pcf_linear");
    color_lighting_[uint8_t(light_type::spot)][uint8_t(sm_depth::linear)][uint8_t(sm_impl::vsm) ].program = loadProgram("vs_clip_quad", "fs_deferred_spot_light_vsm_linear");
    color_lighting_[uint8_t(light_type::spot)][uint8_t(sm_depth::linear)][uint8_t(sm_impl::esm) ].program = loadProgram("vs_clip_quad", "fs_deferred_spot_light_esm_linear");

    color_lighting_no_shadow_[uint8_t(light_type::point)].program = loadProgram("vs_clip_quad", "fs_deferred_point_light");
    color_lighting_[uint8_t(light_type::point)][uint8_t(sm_depth::invz)][uint8_t(sm_impl::hard)].program = loadProgram("vs_clip_quad", "fs_deferred_point_light_hard");
    color_lighting_[uint8_t(light_type::point)][uint8_t(sm_depth::invz)][uint8_t(sm_impl::pcf) ].program = loadProgram("vs_clip_quad", "fs_deferred_point_light_pcf");
    color_lighting_[uint8_t(light_type::point)][uint8_t(sm_depth::invz)][uint8_t(sm_impl::vsm) ].program = loadProgram("vs_clip_quad", "fs_deferred_point_light_vsm");
    color_lighting_[uint8_t(light_type::point)][uint8_t(sm_depth::invz)][uint8_t(sm_impl::esm) ].program = loadProgram("vs_clip_quad", "fs_deferred_point_light_esm");

    color_lighting_[uint8_t(light_type::point)][uint8_t(sm_depth::linear)][uint8_t(sm_impl::hard)].program = loadProgram("vs_clip_quad", "fs_deferred_point_light_hard_linear");
    color_lighting_[uint8_t(light_type::point)][uint8_t(sm_depth::linear)][uint8_t(sm_impl::pcf) ].program = loadProgram("vs_clip_quad", "fs_deferred_point_light_pcf_linear");
    color_lighting_[uint8_t(light_type::point)][uint8_t(sm_depth::linear)][uint8_t(sm_impl::vsm) ].program = loadProgram("vs_clip_quad", "fs_deferred_point_light_vsm_linear");
    color_lighting_[uint8_t(light_type::point)][uint8_t(sm_depth::linear)][uint8_t(sm_impl::esm) ].program = loadProgram("vs_clip_quad", "fs_deferred_point_light_esm_linear");

    color_lighting_no_shadow_[uint8_t(light_type::directional)].program = loadProgram("vs_clip_quad", "fs_deferred_directional_light");
    color_lighting_[uint8_t(light_type::directional)][uint8_t(sm_depth::invz)][uint8_t(sm_impl::hard)].program = loadProgram("vs_clip_quad", "fs_deferred_directional_light_hard");
    color_lighting_[uint8_t(light_type::directional)][uint8_t(sm_depth::invz)][uint8_t(sm_impl::pcf) ].program = loadProgram("vs_clip_quad", "fs_deferred_directional_light_pcf");
    color_lighting_[uint8_t(light_type::directional)][uint8_t(sm_depth::invz)][uint8_t(sm_impl::vsm) ].program = loadProgram("vs_clip_quad", "fs_deferred_directional_light_vsm");
    color_lighting_[uint8_t(light_type::directional)][uint8_t(sm_depth::invz)][uint8_t(sm_impl::esm) ].program = loadProgram("vs_clip_quad", "fs_deferred_directional_light_esm");

    color_lighting_[uint8_t(light_type::directional)][uint8_t(sm_depth::linear)][uint8_t(sm_impl::hard)].program = loadProgram("vs_clip_quad", "fs_deferred_directional_light_hard_linear");
    color_lighting_[uint8_t(light_type::directional)][uint8_t(sm_depth::linear)][uint8_t(sm_impl::pcf) ].program = loadProgram("vs_clip_quad", "fs_deferred_directional_light_pcf_linear");
    color_lighting_[uint8_t(light_type::directional)][uint8_t(sm_depth::linear)][uint8_t(sm_impl::vsm) ].program = loadProgram("vs_clip_quad", "fs_deferred_directional_light_vsm_linear");
    color_lighting_[uint8_t(light_type::directional)][uint8_t(sm_depth::linear)][uint8_t(sm_impl::esm) ].program = loadProgram("vs_clip_quad", "fs_deferred_directional_light_esm_linear");
    // clang-format on

    for(auto& byLightType : color_lighting_no_shadow_)
    {
        if(byLightType.program)
        {
            byLightType.cache_uniforms();
        }
    }
    for(auto& byLightType : color_lighting_)
    {
        for(auto& byDepthType : byLightType)
        {
            for(auto& bySmImpl : byDepthType)
            {
                if(bySmImpl.program)
                {
                    bySmImpl.cache_uniforms();
                }
            }
        }
    }

    ibl_brdf_lut_ = am.get_asset<gfx::texture>("engine:/data/textures/ibl_brdf_lut.png");

    atmospheric_pass_.init(ctx);
    atmospheric_pass_perez_.init(ctx);
    tonemapping_pass_.init(ctx);
    assao_pass_.init(ctx);
    return true;
}

auto deferred::deinit(rtti::context& ctx) -> bool
{
    return true;
}
} // namespace rendering
} // namespace ace
