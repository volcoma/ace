#pragma once

#include <base/basetypes.hpp>
#include <context/context.hpp>
#include <hpp/event.hpp>

namespace ace
{

struct ui_events
{
    hpp::event<void(rtti::context&, delta_t)> on_frame_ui_render;
};

} // namespace ace
