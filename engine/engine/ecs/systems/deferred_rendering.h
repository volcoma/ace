#pragma once
#include "rendering_path.h"

#include <engine/ecs/components/model_component.h>
#include <engine/ecs/components/transform_component.h>
#include <engine/ecs/ecs.h>
#include <engine/rendering/gpu_program.h>

#include "atmospheric_pass.h"
#include "atmospheric_pass_perez.h"

namespace ace
{

class deferred_rendering : public rendering_path
{
public:
    deferred_rendering();
    ~deferred_rendering();

    auto init(rtti::context& ctx) -> bool override;
    auto deinit(rtti::context& ctx) -> bool override;

    auto render_models(camera& camera,
                       gfx::render_view& render_view,
                       ecs& ec,
                       const visibility_set_models_t& visibility_set,
                       lod_data_container& camera_lods,
                       delta_t dt) -> std::shared_ptr<gfx::frame_buffer> override;
    ;

    auto g_buffer_pass(std::shared_ptr<gfx::frame_buffer> input,
                       camera& camera,
                       gfx::render_view& render_view,
                       const visibility_set_models_t& visibility_set,
                       lod_data_container& camera_lods,
                       delta_t dt) -> std::shared_ptr<gfx::frame_buffer>;

    auto lighting_pass(std::shared_ptr<gfx::frame_buffer> input,
                       camera& camera,
                       gfx::render_view& render_view,
                       ecs& ec,
                       delta_t dt) -> std::shared_ptr<gfx::frame_buffer>;

    auto reflection_probe_pass(std::shared_ptr<gfx::frame_buffer> input,
                               camera& camera,
                               gfx::render_view& render_view,
                               ecs& ec,
                               delta_t dt) -> std::shared_ptr<gfx::frame_buffer>;

    auto atmospherics_pass(std::shared_ptr<gfx::frame_buffer> input,
                           camera& camera,
                           gfx::render_view& render_view,
                           ecs& ec,
                           delta_t dt) -> std::shared_ptr<gfx::frame_buffer>;

    auto tonemapping_pass(std::shared_ptr<gfx::frame_buffer> input, camera& camera, gfx::render_view& render_view)
        -> std::shared_ptr<gfx::frame_buffer>;

    void build_reflections_pass(ecs& es, delta_t dt);

    void build_shadows_pass(ecs& es, delta_t dt);

    void on_frame_render(rtti::context& ctx, delta_t dt);

private:
    std::map<entt::handle, lod_data_container> lod_data_;
    /// Program that is responsible for rendering.
    std::unique_ptr<gpu_program> directional_light_program_;
    /// Program that is responsible for rendering.
    std::unique_ptr<gpu_program> point_light_program_;
    /// Program that is responsible for rendering.
    std::unique_ptr<gpu_program> spot_light_program_;
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
