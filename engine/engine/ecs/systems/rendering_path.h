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


class rendering_path
{
public:

    enum visibility_query : uint32_t
    {
        not_specified = 1 << 0,
        dirty = 1 << 1,
        fixed = 1 << 2,
        shadow_caster = 1 << 3,
        reflection_caster = 1 << 4,

    };

    using visibility_flags = uint32_t;

    rendering_path() = default;
    virtual ~rendering_path() = default;

    virtual auto init(rtti::context& ctx) -> bool = 0;
    virtual auto deinit(rtti::context& ctx) -> bool = 0;

    virtual auto gather_visible_models(ecs& es,
                                       camera* camera,
                                       visibility_flags query = visibility_query::fixed) -> visibility_set_models_t;
    virtual auto camera_render_full(camera& camera,
                                    gfx::render_view& render_view,
                                    ecs& ec,
                                    lod_data_container& camera_lods,
                                    delta_t dt) -> std::shared_ptr<gfx::frame_buffer>;

    virtual auto render_models(camera& camera,
                               gfx::render_view& render_view,
                               ecs& ec,
                               const visibility_set_models_t& visibility_set,
                               lod_data_container& camera_lods,
                               delta_t dt) -> std::shared_ptr<gfx::frame_buffer> = 0;
};
} // namespace ace