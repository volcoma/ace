#pragma once

#include <engine/ecs/ecs.h>
#include <engine/rendering/camera.h>

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

struct per_camera_data
{
    lod_data_container entity_lods;
};

class rendering_path
{
public:
    enum visibility_query : uint32_t
    {
        not_specified = 1 << 0,
        is_dirty = 1 << 1,
        is_static = 1 << 2,
        is_shadow_caster = 1 << 3,
        is_reflection_caster = 1 << 4,
    };

    using visibility_flags = uint32_t;

    rendering_path() = default;
    virtual ~rendering_path() = default;

    virtual auto init(rtti::context& ctx) -> bool = 0;
    virtual auto deinit(rtti::context& ctx) -> bool = 0;

    virtual auto gather_visible_models(scene& scn,
                                       const camera* camera,
                                       visibility_flags query = visibility_query::is_static) -> visibility_set_models_t;
    virtual auto camera_render_full(scene& scn,
                                    const camera& camera,
                                    camera_storage& storage,
                                    gfx::render_view& render_view,
                                    delta_t dt,
                                    visibility_flags query = visibility_query::not_specified) -> std::shared_ptr<gfx::frame_buffer> = 0;

    virtual void camera_render_full(const std::shared_ptr<gfx::frame_buffer>& output,
                                    scene& scn,
                                    const camera& camera,
                                    camera_storage& storage,
                                    gfx::render_view& render_view,
                                    delta_t dt,
                                    visibility_flags query = visibility_query::not_specified) = 0;


    virtual void prepare_scene(scene& scn, delta_t dt) = 0;

    auto render_scene(scene& scn, delta_t dt) -> std::shared_ptr<gfx::frame_buffer>;

    void render_scene(const std::shared_ptr<gfx::frame_buffer>& output, scene& scn, delta_t dt);
};
} // namespace ace
