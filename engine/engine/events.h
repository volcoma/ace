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

    /// engine play events
    hpp::event<void(rtti::context&)> on_play_begin;
    hpp::event<void(rtti::context&)> on_play_end;

    hpp::event<void(rtti::context&)> on_pause;
    hpp::event<void(rtti::context&)> on_resume;

    /// os events
    hpp::event<void(rtti::context&, const os::event& e)> on_os_event;

    bool should_quit{};

    void toggle_play_mode(rtti::context& ctx);

    void set_play_mode(rtti::context& ctx, bool play);

    void toggle_pause(rtti::context& ctx);

    void set_paused(rtti::context& ctx, bool paused);

    void skip_next_frame();

    bool is_playing{};
    bool is_paused{};
    bool should_skip_frame{};
};

} // namespace ace
