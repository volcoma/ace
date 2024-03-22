#pragma once

#include "graphics.h"
//
#include "utils/debugdraw/debugdraw.h"

namespace gfx
{
struct dd_raii
{
    dd_raii(view_id _viewId);

    ~dd_raii();

    DebugDrawEncoder encoder;
    view_id view{};
};
} // namespace gfx
