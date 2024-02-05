#pragma once

#include <base/basetypes.hpp>
#include <context/context.hpp>

namespace ace
{
class header_panel
{
public:
    void on_frame_ui_render(rtti::context& ctx, float headerSize);
};
} // namespace ace
