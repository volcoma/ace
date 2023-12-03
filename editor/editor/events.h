#pragma once

#include <base/basetypes.hpp>
#include <context/context.hpp>
#include <hpp/event.hpp>

namespace ace
{

struct ui_events
{
    hpp::event<void(rtti::context&, delta_t)> on_frame_ui_render;

    hpp::event<void(rtti::context&)> on_open_project;
    hpp::event<void(rtti::context&)> on_close_project;

};

} // namespace ace
