#pragma once
#include <editor/imgui/integration/imgui.h>

#include <base/basetypes.hpp>
#include <context/context.hpp>

#include <engine/ecs/ecs.h>

#include "gizmos/gizmos.h"

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

    auto get_camera() -> entt::handle { return panel_camera_; }
private:
    void draw_scene(rtti::context& ctx, delta_t dt);

    void draw_menubar(rtti::context& ctx);

    bool show_statistics_{};
    bool show_gbuffer_{};
    bool enable_profiler_{};


    scene panel_scene_;
    entt::handle panel_camera_{};

    debugdraw_rendering gizmos_{};
};
} // namespace ace
