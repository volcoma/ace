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

    void build_camera_independant_shadows(scene& scn, delta_t dt);
    void build_camera_dependant_shadows(scene& scn, const camera& camera, camera_storage& storage, delta_t dt);

    void on_frame_render(rtti::context& ctx, delta_t dt);

private:
    /// Program that is responsible for rendering.
    std::unique_ptr<gpu_program> box_ref_probe_program_;
    /// Program that is responsible for rendering.
    std::unique_ptr<gpu_program> sphere_ref_probe_program_;
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
