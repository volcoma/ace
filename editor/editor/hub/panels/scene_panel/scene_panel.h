#pragma once
#include <editor/imgui/integration/imgui.h>

#include <base/basetypes.hpp>
#include <context/context.hpp>

#include <engine/ecs/ecs.h>

#include "gizmos/gizmos_renderer.h"

namespace ace
{
class scene_panel
{
public:
    void init(rtti::context& ctx);
    void deinit(rtti::context& ctx);

    void on_frame_update(rtti::context& ctx, delta_t dt);
    void on_frame_render(rtti::context& ctx, delta_t dt);
    void on_frame_ui_render(rtti::context& ctx);

    auto get_camera() -> entt::handle
    {
        return panel_camera_;
    }

    void set_visible(bool visible)
    {
        is_visible_ = visible;
    }

private:
    void draw_scene(rtti::context& ctx, delta_t dt);

    void draw_ui(rtti::context& ctx);
    void draw_menubar(rtti::context& ctx);

    bool is_visible_{};

    bool visualize_passes_{};
    scene panel_scene_;
    entt::handle panel_camera_{};

    gizmos_renderer gizmos_{};
};
} // namespace ace
