#pragma once
#include <editor/imgui/integration/imgui.h>

#include <base/basetypes.hpp>
#include <context/context.hpp>

namespace ace
{
class game_panel
{
public:
    void init(rtti::context& ctx);
    void deinit(rtti::context& ctx);

    void on_frame_update(rtti::context& ctx, delta_t dt);
    void on_frame_render(rtti::context& ctx, delta_t dt);
    void on_frame_ui_render(rtti::context& ctx);
    void set_visible(bool visible) { is_visible_ = visible; }

private:
    void draw_menubar(rtti::context& ctx);

    bool is_visible_{};

};
} // namespace ace
