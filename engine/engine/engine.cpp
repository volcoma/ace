#include "engine.h"
#include "events.h"
#include "rendering/renderer.h"
#include "threading/threader.h"
#include "assets/asset_manager.h"
#include <logging/logging.h>
#include <simulation/simulation.h>

#include <filesystem/filesystem.h>

namespace ace
{

auto engine::create(rtti::context& ctx, cmd_line::parser& parser) -> bool
{

    fs::path binary_path = fs::executable_path(parser.app_name().c_str()).parent_path();
    fs::add_path_protocol("binary:", binary_path);

    fs::path engine_data = binary_path / "data" / "engine";
    fs::add_path_protocol("engine:", engine_data);


    ctx.add<logging>();
    ctx.add<threader>();

    ctx.add<simulation>();
    ctx.add<events>();
    ctx.add<renderer>(ctx, parser);
    ctx.add<asset_manager>(ctx);

    return true;
}

auto engine::init(rtti::context& ctx, const cmd_line::parser& parser) -> bool
{
//    APPLOG_INFO(parser.usage());

    if(!ctx.get<renderer>().init(parser))
    {
        return false;
    }

    if(!ctx.get<asset_manager>().init())
    {
        return false;
    }

    auto& assets = ctx.get<asset_manager>();
    auto testpng = assets.load<gfx::texture>("binary:/data/test.png");
    auto testtga = assets.load<gfx::texture>("binary:/data/test.tga");

    auto png = testpng.get();

    auto tga = testtga.get();

    assets.rename_asset<gfx::texture>("binary:/data/test.png", "binary:/data/test2.png");

    return true;
}

auto engine::deinit(rtti::context& ctx) -> bool
{
    ctx.remove<asset_manager>();

    ctx.remove<renderer>();
    ctx.remove<events>();
    ctx.remove<simulation>();

    ctx.remove<threader>();
    ctx.remove<logging>();

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

    const auto& windows = rend.get_windows();

    bool should_quit = windows.empty();
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
