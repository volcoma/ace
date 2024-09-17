#include "engine.h"
#include <engine/assets/asset_manager.h>
#include <engine/defaults/defaults.h>
#include <engine/profiler/profiler.h>
#include <engine/rendering/renderer.h>
#include <engine/scripting/script_system.h>
#include <engine/threading/threader.h>

#include <engine/ecs/ecs.h>

#include <engine/audio/ecs/systems/audio_system.h>
#include <engine/ecs/systems/bone_system.h>
#include <engine/animation/ecs/systems/animation_system.h>
#include <engine/ecs/systems/transform_system.h>
#include <engine/ecs/systems/camera_system.h>
#include <engine/ecs/systems/reflection_probe_system.h>
#include <engine/ecs/systems/rendering_path.h>
#include <engine/physics/ecs/systems/physics_system.h>

#include "events.h"
#include <ospp/event.h>

#include <logging/logging.h>
#include <simulation/simulation.h>

#include <filesystem/filesystem.h>

namespace ace
{
namespace
{
auto context_ptr() -> rtti::context*&
{
    static rtti::context* ctx{};
    return ctx;
}
} // namespace

auto engine::context() -> rtti::context&
{
    return *context_ptr();
}

auto engine::create(rtti::context& ctx, cmd_line::parser& parser) -> bool
{
    context_ptr() = &ctx;

    fs::path binary_path = fs::executable_path(parser.app_name().c_str()).parent_path();
    fs::add_path_protocol("binary", binary_path);

    fs::path engine_data = binary_path / "data" / "engine";
    fs::add_path_protocol("engine", engine_data);

    serialization::set_warning_logger(
        [](const std::string& log, const hpp::source_location& loc)
        {
            APPLOG_WARNING_LOC(loc.file_name(), int(loc.line()), loc.function_name(), "Serialization {}", log);
        });

    ctx.add<logging>();
    ctx.add<simulation>();
    ctx.add<events>();
    ctx.add<threader>();
    ctx.add<renderer>(ctx, parser);
    ctx.add<audio_system>();
    ctx.add<asset_manager>(ctx);
    ctx.add<script_system>();
    ctx.add<ecs>();
    ctx.add<rendering_path>();
    ctx.add<transform_system>();
    ctx.add<camera_system>();
    ctx.add<reflection_probe_system>();
    ctx.add<bone_system>();
    ctx.add<animation_system>();
    ctx.add<physics_system>();

    return true;
}

auto engine::init_core(const cmd_line::parser& parser) -> bool
{
    auto& ctx = engine::context();

    //    APPLOG_INFO(parser.usage());

    if(!ctx.get<threader>().init(ctx))
    {
        return false;
    }

    if(!ctx.get<renderer>().init(ctx, parser))
    {
        return false;
    }

    if(!ctx.get<audio_system>().init(ctx))
    {
        return false;
    }

    if(!ctx.get<asset_manager>().init(ctx))
    {
        return false;
    }

    if(!ctx.get<script_system>().init(ctx))
    {
        return false;
    }

    return true;
}

auto engine::init_systems(const cmd_line::parser& parser) -> bool
{
    auto& ctx = engine::context();

    if(!ctx.get<ecs>().init(ctx))
    {
        return false;
    }

    if(!ctx.get<rendering_path>().init(ctx))
    {
        return false;
    }

    if(!ctx.get<transform_system>().init(ctx))
    {
        return false;
    }

    if(!ctx.get<camera_system>().init(ctx))
    {
        return false;
    }

    if(!ctx.get<reflection_probe_system>().init(ctx))
    {
        return false;
    }

    if(!ctx.get<bone_system>().init(ctx))
    {
        return false;
    }

    if(!ctx.get<animation_system>().init(ctx))
    {
        return false;
    }

    if(!ctx.get<physics_system>().init(ctx))
    {
        return false;
    }

    if(!defaults::init(ctx))
    {
        return false;
    }

    return true;
}

auto engine::deinit() -> bool
{
    auto& ctx = engine::context();

    if(!defaults::deinit(ctx))
    {
        return false;
    }

    if(!ctx.get<physics_system>().deinit(ctx))
    {
        return false;
    }

    if(!ctx.get<animation_system>().deinit(ctx))
    {
        return false;
    }

    if(!ctx.get<bone_system>().deinit(ctx))
    {
        return false;
    }

    if(!ctx.get<reflection_probe_system>().deinit(ctx))
    {
        return false;
    }

    if(!ctx.get<camera_system>().deinit(ctx))
    {
        return false;
    }

    if(!ctx.get<transform_system>().deinit(ctx))
    {
        return false;
    }

    if(!ctx.get<rendering_path>().deinit(ctx))
    {
        return false;
    }

    if(!ctx.get<ecs>().deinit(ctx))
    {
        return false;
    }

    if(!ctx.get<asset_manager>().deinit(ctx))
    {
        return false;
    }

    if(!ctx.get<script_system>().deinit(ctx))
    {
        return false;
    }

    if(!ctx.get<audio_system>().deinit(ctx))
    {
        return false;
    }

    if(!ctx.get<renderer>().deinit(ctx))
    {
        return false;
    }

    if(!ctx.get<threader>().deinit(ctx))
    {
        return false;
    }

    return true;
}

auto engine::destroy() -> bool
{
    auto& ctx = engine::context();

    ctx.remove<defaults>();
    ctx.remove<physics_system>();
    ctx.remove<animation_system>();
    ctx.remove<bone_system>();
    ctx.remove<reflection_probe_system>();
    ctx.remove<camera_system>();
    ctx.remove<transform_system>();
    ctx.remove<rendering_path>();
    ctx.remove<ecs>();
    ctx.remove<script_system>();

    ctx.remove<asset_manager>();
    ctx.remove<audio_system>();
    ctx.remove<renderer>();
    ctx.remove<events>();
    ctx.remove<simulation>();
    ctx.remove<threader>();
    ctx.remove<logging>();

    bool empty = ctx.empty();
    if(!empty)
    {
        ctx.print_types();
    }

    context_ptr() = {};
    return !empty;
}

auto engine::process() -> bool
{
    auto& ctx = engine::context();

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
        ev.set_play_mode(ctx, false);
        return false;
    }

    ev.on_frame_begin(ctx, dt);

    ev.on_frame_update(ctx, dt);

    ev.on_frame_render(ctx, dt);

    ev.on_frame_end(ctx, dt);

    get_app_profiler()->swap();

    return true;
}

} // namespace ace
