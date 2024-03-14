#include "game.h"

#include <engine/engine.h>
#include <engine/events.h>
#include <engine/rendering/renderer.h>

#include "runner/runner.h"

#include <filesystem/filesystem.h>
#include <logging/logging.h>
#include <rttr/registration>

namespace ace
{
RTTR_REGISTRATION
{
    rttr::registration::class_<game>("game")
        .constructor<>()
        .method("create", &game::create)
        .method("init", &game::init)
        .method("deinit", &game::deinit)
        .method("destroy", &game::destroy)
        .method("process", &game::process);
}

auto game::create(rtti::context& ctx, cmd_line::parser& parser) -> bool
{
    if(!engine::create(ctx, parser))
    {
        return false;
    }

    ctx.add<runner>();

    fs::path binary_path = fs::resolve_protocol("binary:/");
    fs::path app_data = binary_path / "data" / "app";
    fs::add_path_protocol("app", app_data);

    return true;
}

auto game::init(const cmd_line::parser& parser) -> bool
{
    if(!engine::init_core(parser))
    {
        return false;
    }

    auto& ctx = engine::context();

    if(!init_window(ctx))
    {
        return false;
    }

    if(!engine::init_systems(parser))
    {
        return false;
    }

    if(!ctx.get<runner>().init(ctx))
    {
        return false;
    }

    auto& ev = ctx.get<events>();
    ev.set_play_mode(ctx, true);

    return true;
}

auto game::init_window(rtti::context& ctx) -> bool
{
    auto title = fmt::format("Ace Game <{}>", gfx::get_renderer_name(gfx::get_renderer_type()));
    uint32_t flags = os::window::resizable | os::window::maximized;
    auto primary_display = os::display::get_primary_display_index();

    auto& rend = ctx.get<renderer>();
    rend.create_window_for_display(primary_display, title, flags);
    return true;
}


auto game::deinit() -> bool
{
    auto& ctx = engine::context();

    if(!ctx.get<runner>().deinit(ctx))
    {
        return false;
    }


    return engine::deinit();
}

auto game::destroy() -> bool
{
    auto& ctx = engine::context();

    ctx.remove<runner>();

    return engine::destroy();
}

auto game::process() -> bool
{
    if(!engine::process())
    {
        return false;
    }

    return true;
}
} // namespace ace
