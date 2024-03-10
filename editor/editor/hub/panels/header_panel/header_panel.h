#pragma once

#include <base/basetypes.hpp>
#include <context/context.hpp>

namespace ace
{

class imgui_panels;

class header_panel
{
public:
    header_panel(imgui_panels* parent);

    void on_frame_ui_render(rtti::context& ctx, float headerSize);

private:
    void draw_menubar_child(rtti::context& ctx);
    void draw_play_toolbar(rtti::context& ctx, float headerSize);

    imgui_panels* parent_{};
};
} // namespace ace
