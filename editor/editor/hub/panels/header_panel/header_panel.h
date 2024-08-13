#pragma once

#include <base/basetypes.hpp>
#include <context/context.hpp>

#include <editor/imgui/integration/imgui.h>

namespace ace
{

class imgui_panels;

class header_panel
{
public:
    header_panel(imgui_panels* parent);

    void on_frame_ui_render(rtti::context& ctx, float header_size);

private:
    void draw_menubar_child(rtti::context& ctx);
    void draw_play_toolbar(rtti::context& ctx, float header_size);

    imgui_panels* parent_{};

    ImGuiKeyCombination new_scene_key_{ImGuiKey_LeftCtrl, ImGuiKey_N};
    ImGuiKeyCombination open_scene_key_{ImGuiKey_LeftCtrl, ImGuiKey_O};
    ImGuiKeyCombination save_scene_key_{ImGuiKey_LeftCtrl, ImGuiKey_S};
    ImGuiKeyCombination save_scene_as_key_{ImGuiKey_LeftCtrl, ImGuiKey_LeftShift, ImGuiKey_S};
};
} // namespace ace
