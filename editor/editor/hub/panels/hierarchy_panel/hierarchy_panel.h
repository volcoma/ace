#pragma once
#include <editor/imgui/integration/imgui.h>

#include <base/basetypes.hpp>
#include <context/context.hpp>
#include "../entity_panel.h"

namespace ace
{
class hierarchy_panel : public entity_panel
{
public:
    hierarchy_panel(imgui_panels* parent);

    void init(rtti::context& ctx);

    void on_frame_ui_render(rtti::context& ctx, const char* name);

private:
};
} // namespace ace
