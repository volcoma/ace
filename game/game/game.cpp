#include "game.h"

#include <engine/engine.h>
#include <engine/events.h>
#include <engine/rendering/renderer.h>

#include <filesystem/filesystem.h>
#include <logging/logging.h>
#include <rttr/registration>

namespace ace
{
RTTR_PLUGIN_REGISTRATION
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

    fs::path binary_path = fs::resolve_protocol("binary:/");
    fs::path editor_data = binary_path / "data" / "editor";
    fs::add_path_protocol("editor", editor_data);

    return true;
}

auto game::init(rtti::context& ctx, const cmd_line::parser& parser) -> bool
{
    if(!engine::init(ctx, parser))
    {
        return false;
    }

    auto& rend = ctx.get<renderer>();
    auto& win = rend.get_main_window();

    win->get_window().set_title(fmt::format("Ace Game <{}>", gfx::get_renderer_name(gfx::get_renderer_type())));

    return true;
}

auto game::deinit(rtti::context& ctx) -> bool
{
    return engine::deinit(ctx);
}

auto game::destroy(rtti::context& ctx) -> bool
{
    return engine::destroy(ctx);
}

auto game::process(rtti::context& ctx) -> bool
{
    if(!engine::process(ctx))
    {
        return false;
    }

    return true;
}
} // namespace ace
