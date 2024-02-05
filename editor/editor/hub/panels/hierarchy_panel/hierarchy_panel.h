#pragma once
#include <editor/imgui/integration/imgui.h>

#include <base/basetypes.hpp>
#include <context/context.hpp>

#include "../scene_panel/scene_panel.h"

namespace ace
{
class hierarchy_panel
{
public:
    void init(rtti::context& ctx);

    void on_frame_ui_render(rtti::context& ctx, scene_panel* scene_pnl);

private:
};
} // namespace ace
