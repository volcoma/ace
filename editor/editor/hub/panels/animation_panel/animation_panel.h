#pragma once
#include <editor/imgui/integration/imgui.h>
#include <imgui_widgets/flow/ImNodeFlow.h>

namespace ace
{
class imgui_panels;

class animation_panel
{
public:
    animation_panel(imgui_panels* parent);

    void init(rtti::context& ctx);
    void deinit(rtti::context& ctx);

    void on_frame_ui_render(rtti::context& ctx, const char* name);

    void show(bool s);

private:

    void draw_ui(rtti::context& ctx);
    void draw_menubar(rtti::context& ctx);

    imgui_panels* parent_{};
    bool show_request_{};
    bool show_{};
    ImFlow::ImNodeFlow flow_;
    ImGuiTextFilter filter_;

};
} // namespace ace
