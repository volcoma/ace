#include "engine.h"
#include "events.h"
#include "rendering/renderer.h"

#include <logging/logging.h>
#include <simulation/simulation.h>

#include <chrono>
#include <iostream>
#include <thread>

namespace ace
{
using namespace std::chrono_literals;

auto engine::create(rtti::context& ctx, cmd_line::parser& parser) -> bool
{
    ctx.add<logging>();

    ctx.add<simulation>();
    ctx.add<events>();
    ctx.add<renderer>(ctx, parser);

	return true;
}

auto engine::init(rtti::context& ctx, const cmd_line::parser& parser) -> bool
{
    if(!ctx.get<renderer>().init(parser))
    {
        return false;
    }

	return true;
}

auto engine::deinit(rtti::context& ctx) -> bool
{
    ctx.remove<renderer>();
	ctx.remove<events>();
    ctx.remove<simulation>();

    ctx.remove<logging>();

	return true;
}

auto engine::process(rtti::context& ctx) -> bool
{
    auto& sim = ctx.get<simulation>();
    auto& ev = ctx.get<events>();
    auto& rend = ctx.get<renderer>();

    const bool is_active = true;//rend.get_focused_window() != nullptr;

    sim.run_one_frame(is_active);

    auto dt = sim.get_delta_time();

    os::event e{};
    while(os::poll_event(e))
    {
        ev.on_os_event(ctx, e);
    }

    const auto& windows = rend.get_windows();

    bool should_quit = windows.empty();
    if(should_quit)
    {
        return false;
    }

    ev.on_frame_begin(ctx, dt);

    ev.on_frame_update(ctx, dt);

    ev.on_frame_render(ctx, dt);

    ev.on_frame_ui_render(ctx, dt);

    ev.on_frame_end(ctx, dt);

	return true;
}

} // namespace ace
