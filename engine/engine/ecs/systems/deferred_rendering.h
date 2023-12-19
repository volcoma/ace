#pragma once

#include <engine/ecs/components/model_component.h>
#include <engine/ecs/components/transform_component.h>
#include <engine/ecs/ecs.h>
#include <engine/rendering/camera.h>
#include <engine/rendering/gpu_program.h>

#include <base/basetypes.hpp>
#include <context/context.hpp>

#include <memory>
#include <vector>

namespace gfx
{
struct texture;
struct frame_buffer;
class render_view;
} // namespace gfx

namespace ace
{
struct lod_data
{
    std::uint32_t current_lod_index = 0;
    std::uint32_t target_lod_index = 0;
    float current_time = 0.0f;
};
using lod_data_container = std::map<entt::handle, lod_data>;

using visibility_set_models_t = std::vector<entt::handle>;

class deferred_rendering
{
public:
    deferred_rendering();
    ~deferred_rendering();

    auto init(rtti::context& ctx) -> bool;
    auto deinit(rtti::context& ctx) -> bool;

    auto gather_visible_models(ecs& es,
                               camera* camera,
                               bool dirty_only = false,
                               bool static_only = true,
                               bool require_reflection_caster = false) -> visibility_set_models_t;

    void on_frame_render(rtti::context& ctx, delta_t dt);

    void build_reflections_pass(ecs& es, delta_t dt);

    void build_shadows_pass(ecs& es, delta_t dt);

    auto camera_render_full(camera& camera,
                            gfx::render_view& render_view,
                            ecs& ec,
                            lod_data_container& camera_lods,
                            delta_t dt) -> std::shared_ptr<gfx::frame_buffer>;

    auto render_models(camera& camera,
                       gfx::render_view& render_view,
                       ecs& ec,
                       const visibility_set_models_t& visibility_set,
                       lod_data_container& camera_lods,
                       delta_t dt) -> std::shared_ptr<gfx::frame_buffer>;

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
    ///
    asset_handle<gfx::texture> ibl_brdf_lut_;

    std::shared_ptr<int> sentinel_ = std::make_shared<int>(0);
};
} // namespace ace
