#pragma once

#include <base/basetypes.hpp>
#include <context/context.hpp>
#include <hpp/event.hpp>
#include <ospp/event.h>

namespace ace
{

struct events
{
    /// engine loop events
    hpp::event<void(rtti::context&, delta_t)> on_frame_begin;
    hpp::event<void(rtti::context&, delta_t)> on_frame_update;
    hpp::event<void(rtti::context&, delta_t)> on_frame_render;
    hpp::event<void(rtti::context&, delta_t)> on_frame_end;

    /// os events
    hpp::event<void(rtti::context&, const os::event& e)> on_os_event;

    bool should_quit{};
};

} // namespace ace
