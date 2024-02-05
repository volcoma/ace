#pragma once

namespace ace
{
class dockspace
{
public:
    void on_frame_ui_render(float headerSize, float footerSize);

    void execute_dock_builder_order_and_focus_workaround();
};
} // namespace ace
