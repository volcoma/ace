#pragma once
#include "rendering_path.h"

#include <engine/ecs/components/model_component.h>
#include <engine/ecs/components/transform_component.h>
#include <engine/ecs/ecs.h>
#include <engine/rendering/gpu_program.h>
#include <engine/rendering/light.h>

#include "atmospheric_pass.h"
#include "atmospheric_pass_perez.h"
// #include "shadows_rendering.h"

namespace ace
{

class deferred_rendering : public rendering_path
{
public:
    deferred_rendering();
    ~deferred_rendering();

    auto init(rtti::context& ctx) -> bool override;
    auto deinit(rtti::context& ctx) -> bool override;

    void prepare_scene(scene& scn, delta_t dt) override;

    auto camera_render_full(scene& scn,
                            const camera& camera,
                            camera_storage& storage,
                            gfx::render_view& render_view,
                            delta_t dt,
                            visibility_flags query = visibility_query::not_specified)
        -> std::shared_ptr<gfx::frame_buffer> override;

    void camera_render_full(const std::shared_ptr<gfx::frame_buffer>& output,
                            scene& scn,
                            const camera& camera,
                            camera_storage& storage,
                            gfx::render_view& render_view,
                            delta_t dt,
                            visibility_flags query = visibility_query::not_specified) override;

    enum pipeline_steps : uint32_t
    {
        geometry_pass = 1 << 1,
        shadow_pass = 1 << 2,
        reflection_probe = 1 << 3,
        lighting = 1 << 4,
        atmospheric = 1 << 5,

        full = geometry_pass | shadow_pass| reflection_probe | lighting | atmospheric,
        probe = lighting | atmospheric,
    };
    using pipeline_flags = uint32_t;

    auto run_pipeline(pipeline_flags pipeline,
                      scene& scn,
                      const camera& camera,
                      gfx::render_view& render_view,
                      delta_t dt,
                      visibility_flags query = visibility_query::not_specified) -> std::shared_ptr<gfx::frame_buffer>;

    void run_pipeline(pipeline_flags pipeline,
                      const std::shared_ptr<gfx::frame_buffer>& output,
                      scene& scn,
                      const camera& camera,
                      gfx::render_view& render_view,
                      delta_t dt,
                      visibility_flags query = visibility_query::not_specified);


    auto g_buffer_pass(std::shared_ptr<gfx::frame_buffer> input,
                       const visibility_set_models_t& visibility_set,
                       const camera& camera,
                       gfx::render_view& render_view,
                       delta_t dt) -> std::shared_ptr<gfx::frame_buffer>;

    auto lighting_pass(std::shared_ptr<gfx::frame_buffer> input,
                       scene& scn,
                       const camera& camera,
                       gfx::render_view& render_view,
                       bool apply_shadows,
                       delta_t dt) -> std::shared_ptr<gfx::frame_buffer>;

    auto reflection_probe_pass(std::shared_ptr<gfx::frame_buffer> input,
                               scene& scn,
                               const camera& camera,
                               gfx::render_view& render_view,
                               delta_t dt) -> std::shared_ptr<gfx::frame_buffer>;

    auto atmospherics_pass(std::shared_ptr<gfx::frame_buffer> input,
                           scene& scn,
                           const camera& camera,
                           gfx::render_view& render_view,
                           delta_t dt) -> std::shared_ptr<gfx::frame_buffer>;

    auto tonemapping_pass(std::shared_ptr<gfx::frame_buffer> input, const camera& camera, gfx::render_view& render_view)
        -> std::shared_ptr<gfx::frame_buffer>;

    void tonemapping_pass(std::shared_ptr<gfx::frame_buffer> input, std::shared_ptr<gfx::frame_buffer> output);

    void build_camera_independant_reflections(scene& scn, delta_t dt);

    void build_camera_independant_shadows(scene& scn, visibility_flags query = visibility_query::not_specified);
    void build_camera_dependant_shadows(scene& scn, const camera& camera, visibility_flags query = visibility_query::not_specified);
    void build_shadows(scene& scn, const camera* camera, visibility_flags query = visibility_query::not_specified);

    void on_frame_render(rtti::context& ctx, delta_t dt);

private:
    struct ref_probe_program : uniforms_cache
    {
        void cache_uniforms()
        {
            cache_uniform(program.get(), u_data0, "u_data0");
            cache_uniform(program.get(), u_data1, "u_data1");
            cache_uniform(program.get(), s_tex[0], "s_tex0");
            cache_uniform(program.get(), s_tex[1], "s_tex1");
            cache_uniform(program.get(), s_tex[2], "s_tex2");
            cache_uniform(program.get(), s_tex[3], "s_tex3");
            cache_uniform(program.get(), s_tex[4], "s_tex4");
            cache_uniform(program.get(), s_tex_cube, "s_tex_cube");
        }

        gfx::program::uniform_ptr u_data0;
        gfx::program::uniform_ptr u_data1;

        std::array<gfx::program::uniform_ptr, 5> s_tex;
        gfx::program::uniform_ptr s_tex_cube;

        std::unique_ptr<gpu_program> program;
    };

    struct box_ref_probe_program : ref_probe_program
    {
        void cache_uniforms()
        {
            ref_probe_program::cache_uniforms();

            cache_uniform(program.get(), u_data2, "u_data2");
            cache_uniform(program.get(), u_inv_world, "u_inv_world");
        }
        gfx::program::uniform_ptr u_inv_world;
        gfx::program::uniform_ptr u_data2;

    } box_ref_probe_program_;

    struct sphere_ref_probe_program : ref_probe_program
    {
    } sphere_ref_probe_program_;

    struct gamma_correction_program : uniforms_cache
    {
        void cache_uniforms()
        {
            cache_uniform(program.get(), s_input, "s_input");
        }

        gfx::program::uniform_ptr s_input;
        std::unique_ptr<gpu_program> program;

    } gamma_correction_program_;

    struct geom_program : uniforms_cache
    {
        void cache_uniforms()
        {
            cache_uniform(program.get(), s_tex_color, "s_tex_color");
            cache_uniform(program.get(), s_tex_normal, "s_tex_normal");
            cache_uniform(program.get(), s_tex_roughness, "s_tex_roughness");
            cache_uniform(program.get(), s_tex_metalness, "s_tex_metalness");
            cache_uniform(program.get(), s_tex_ao, "s_tex_ao");
            cache_uniform(program.get(), s_tex_emissive, "s_tex_emissive");

            cache_uniform(program.get(), u_base_color, "u_base_color");
            cache_uniform(program.get(), u_subsurface_color, "u_subsurface_color");
            cache_uniform(program.get(), u_emissive_color, "u_emissive_color");
            cache_uniform(program.get(), u_surface_data, "u_surface_data");
            cache_uniform(program.get(), u_tiling, "u_tiling");
            cache_uniform(program.get(), u_dither_threshold, "u_dither_threshold");
            cache_uniform(program.get(), u_surface_data2, "u_surface_data2");

            cache_uniform(program.get(), u_camera_wpos, "u_camera_wpos");
            cache_uniform(program.get(), u_camera_clip_planes, "u_camera_clip_planes");
            cache_uniform(program.get(), u_lod_params, "u_lod_params");
        }

        gfx::program::uniform_ptr s_tex_color;
        gfx::program::uniform_ptr s_tex_normal;
        gfx::program::uniform_ptr s_tex_roughness;
        gfx::program::uniform_ptr s_tex_metalness;
        gfx::program::uniform_ptr s_tex_ao;
        gfx::program::uniform_ptr s_tex_emissive;

        gfx::program::uniform_ptr u_base_color;
        gfx::program::uniform_ptr u_subsurface_color;
        gfx::program::uniform_ptr u_emissive_color;
        gfx::program::uniform_ptr u_surface_data;
        gfx::program::uniform_ptr u_tiling;
        gfx::program::uniform_ptr u_dither_threshold;
        gfx::program::uniform_ptr u_surface_data2;

        gfx::program::uniform_ptr u_camera_wpos;
        gfx::program::uniform_ptr u_camera_clip_planes;
        gfx::program::uniform_ptr u_lod_params;

        std::unique_ptr<gpu_program> program;
    };

    geom_program geom_program_;
    geom_program geom_program_skinned_;

    struct color_lighting : uniforms_cache
    {
        void cache_uniforms()
        {
            cache_uniform(program.get(), u_light_position, "u_light_position");
            cache_uniform(program.get(), u_light_direction, "u_light_direction");
            cache_uniform(program.get(), u_light_data, "u_light_data");
            cache_uniform(program.get(), u_light_color_intensity, "u_light_color_intensity");
            cache_uniform(program.get(), u_camera_position, "u_camera_position");

            cache_uniform(program.get(), s_tex0, "s_tex0");
            cache_uniform(program.get(), s_tex1, "s_tex1");
            cache_uniform(program.get(), s_tex2, "s_tex2");
            cache_uniform(program.get(), s_tex3, "s_tex3");
            cache_uniform(program.get(), s_tex4, "s_tex4");
            cache_uniform(program.get(), s_tex5, "s_tex5");
            cache_uniform(program.get(), s_tex6, "s_tex6");
        }
        gfx::program::uniform_ptr u_light_position;
        gfx::program::uniform_ptr u_light_direction;
        gfx::program::uniform_ptr u_light_data;
        gfx::program::uniform_ptr u_light_color_intensity;
        gfx::program::uniform_ptr u_camera_position;
        gfx::program::uniform_ptr s_tex0;
        gfx::program::uniform_ptr s_tex1;
        gfx::program::uniform_ptr s_tex2;
        gfx::program::uniform_ptr s_tex3;
        gfx::program::uniform_ptr s_tex4;
        gfx::program::uniform_ptr s_tex5;
        gfx::program::uniform_ptr s_tex6;

        std::shared_ptr<gpu_program> program;
    };

    auto get_light_program(const light& l) const -> const color_lighting&;
    auto get_light_program_no_shadows(const light& l) const -> const color_lighting&;

    color_lighting color_lighting_[uint8_t(light_type::count)][uint8_t(sm_depth::count)][uint8_t(sm_impl::count)];
    color_lighting color_lighting_no_shadow_[uint8_t(light_type::count)];

    asset_handle<gfx::texture> ibl_brdf_lut_;

    atmospheric_pass atmospheric_pass_{};
    atmospheric_pass_perez atmospheric_pass_perez_{};

    void submit_material(geom_program& program, const pbr_material& mat);

    std::shared_ptr<int> sentinel_ = std::make_shared<int>(0);
};
} // namespace ace
