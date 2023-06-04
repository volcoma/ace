#pragma once

#include <context/context.hpp>
#include <base/basetypes.hpp>
#include <graphics/texture.h>

#include "../integration/imgui.h"


namespace ace
{

class imgui_panels
{
public:
    void setup_panels(rtti::context& ctx, ImGuiID dockspace_id);

    void draw(rtti::context& ctx);

    void draw_panels(rtti::context& ctx);
};
} // namespace ace
