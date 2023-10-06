#pragma once

#include <base/basetypes.hpp>
#include <context/context.hpp>
#include <graphics/texture.h>

#include "console_log/console_log.h"

namespace ace
{

class imgui_panels
{
public:
    imgui_panels();
    ~imgui_panels();

    void setup_panels(rtti::context& ctx, ImGuiID dockspace_id);

    void draw(rtti::context& ctx);

    void draw_panels(rtti::context& ctx);

private:
    std::shared_ptr<console_log> console_log_;
};
} // namespace ace
