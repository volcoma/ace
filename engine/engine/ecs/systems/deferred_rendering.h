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

    auto build_per_camera_data(scene& scn,
                               const camera& camera,
                               camera_storage& storage,
                               gfx::render_view& render_view,
                               delta_t dt) -> per_camera_data& override;

    auto render_models(const visibility_set_models_t& visibility_set,
                       scene& scn,
                       const camera& camera,
                       camera_storage& storage,
                       gfx::render_view& render_view,
                       delta_t dt) -> std::shared_ptr<gfx::frame_buffer> override;

    void render_models(const std::shared_ptr<gfx::frame_buffer>& output,
                       const visibility_set_models_t& visibility_set,
                       scene& scn,
                       const camera& camera,
                       camera_storage& storage,
                       gfx::render_view& render_view,
                       delta_t dt) override;

    auto g_buffer_pass(std::shared_ptr<gfx::frame_buffer> input,
                       const visibility_set_models_t& visibility_set,
                       const camera& camera,
                       gfx::render_view& render_view,
                       delta_t dt) -> std::shared_ptr<gfx::frame_buffer>;

    auto lighting_pass(std::shared_ptr<gfx::frame_buffer> input,
                       scene& scn,
                       const camera& camera,
                       gfx::render_view& render_view,
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

    void build_camera_independant_shadows(scene& scn);
    void build_camera_dependant_shadows(scene& scn, const camera& camera, camera_storage& storage);
    void build_shadows(scene& scn, const camera* camera);

    void on_frame_render(rtti::context& ctx, delta_t dt);

    // void submit_material(gpu_program& program, const pbr_material* mat);

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

    /// Program that is responsible for rendering.
    std::unique_ptr<gpu_program> gamma_correction_program_;
    /// Program that is responsible for rendering.
    std::unique_ptr<gpu_program> atmospherics_program_;

    std::unique_ptr<gpu_program> geom_program_;
    std::unique_ptr<gpu_program> geom_skinned_program_;

    asset_handle<gfx::texture> ibl_brdf_lut_;

    atmospheric_pass atmospheric_pass_{};
    atmospheric_pass_perez atmospheric_pass_perez_{};

    std::shared_ptr<int> sentinel_ = std::make_shared<int>(0);
};
} // namespace ace
