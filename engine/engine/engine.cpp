#include "engine.h"
#include "assets/asset_manager.h"
#include "assets/asset_watcher.h"
#include "defaults/defaults.h"
#include "ecs/ecs.h"
#include "ecs/systems/deferred_rendering.h"

#include "events.h"
#include "meta/meta.h"
#include "rendering/renderer.h"
#include "threading/threader.h"
#include <logging/logging.h>
#include <simulation/simulation.h>

#include <filesystem/filesystem.h>

namespace ace
{

auto engine::create(rtti::context& ctx, cmd_line::parser& parser) -> bool
{
    fs::path binary_path = fs::executable_path(parser.app_name().c_str()).parent_path();
    fs::add_path_protocol("binary", binary_path);

    fs::path engine_data = binary_path / "data" / "engine";
    fs::add_path_protocol("engine", engine_data);

    ctx.add<logging>();
    ctx.add<simulation>();
    ctx.add<events>();
    ctx.add<threader>();
    ctx.add<renderer>(ctx, parser);
    ctx.add<meta>();
    ctx.add<asset_manager>(ctx);
    ctx.add<asset_watcher>();
    ctx.add<defaults>();
    ctx.add<ecs>();
    ctx.add<deferred_rendering>();

    return true;
}

auto engine::init(rtti::context& ctx, const cmd_line::parser& parser) -> bool
{
    //    APPLOG_INFO(parser.usage());

    if(!ctx.get<threader>().init(ctx))
    {
        return false;
    }

    if(!ctx.get<renderer>().init(ctx, parser))
    {
        return false;
    }

    if(!ctx.get<meta>().init(ctx))
    {
        return false;
    }

    if(!ctx.get<asset_manager>().init(ctx))
    {
        return false;
    }

    if(!ctx.get<asset_watcher>().init(ctx))
    {
        return false;
    }

    if(!ctx.get<ecs>().init(ctx))
    {
        return false;
    }

    if(!ctx.get<deferred_rendering>().init(ctx))
    {
        return false;
    }

    if(!ctx.get<defaults>().init(ctx))
    {
        return false;
    }

    return true;
}

auto engine::deinit(rtti::context& ctx) -> bool
{
    if(!ctx.get<defaults>().deinit(ctx))
    {
        return false;
    }

    if(!ctx.get<deferred_rendering>().deinit(ctx))
    {
        return false;
    }

    if(!ctx.get<ecs>().deinit(ctx))
    {
        return false;
    }

    if(!ctx.get<asset_watcher>().deinit(ctx))
    {
        return false;
    }

    if(!ctx.get<asset_manager>().deinit(ctx))
    {
        return false;
    }

    if(!ctx.get<renderer>().deinit(ctx))
    {
        return false;
    }

    if(!ctx.get<meta>().deinit(ctx))
    {
        return false;
    }

    if(!ctx.get<threader>().deinit(ctx))
    {
        return false;
    }

    return true;
}

auto engine::destroy(rtti::context& ctx) -> bool
{
    ctx.remove<defaults>();
    ctx.remove<deferred_rendering>();
    ctx.remove<ecs>();
    ctx.remove<asset_watcher>();
    ctx.remove<asset_manager>();
    ctx.remove<renderer>();
    ctx.remove<events>();
    ctx.remove<simulation>();
    ctx.remove<threader>();
    ctx.remove<meta>();
    ctx.remove<logging>();

    if(!ctx.empty())
    {
        ctx.print_types();
        return false;
    }
    return true;
}

auto engine::process(rtti::context& ctx) -> bool
{
    auto& sim = ctx.get<simulation>();
    auto& ev = ctx.get<events>();
    auto& rend = ctx.get<renderer>();
    auto& thr = ctx.get<threader>();

    thr.process();

    sim.run_one_frame(true);

    auto dt = sim.get_delta_time();

    os::event e{};
    while(os::poll_event(e))
    {
        ev.on_os_event(ctx, e);
    }

    const auto& window = rend.get_main_window();

    bool should_quit = window == nullptr;

    if(should_quit)
    {
        return false;
    }

    ev.on_frame_begin(ctx, dt);

    ev.on_frame_update(ctx, dt);

    ev.on_frame_render(ctx, dt);

    ev.on_frame_end(ctx, dt);

    return true;
}

} // namespace ace
