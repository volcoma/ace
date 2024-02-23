#include "editor.h"

#include <engine/engine.h>
#include <engine/events.h>
#include <engine/rendering/renderer.h>

#include <rttr/registration>

#include "editing/editing_manager.h"
#include "editing/picking_manager.h"
#include "editing/thumbnail_manager.h"
#include "events.h"
#include "hub/hub.h"
#include "imgui/imgui_interface.h"
#include "system/project_manager.h"

namespace ace
{
RTTR_PLUGIN_REGISTRATION
{
    rttr::registration::class_<editor>("editor")
        .constructor<>()
        .method("create", &editor::create)
        .method("init", &editor::init)
        .method("deinit", &editor::deinit)
        .method("destroy", &editor::destroy)
        .method("process", &editor::process);
}

auto editor::create(rtti::context& ctx, cmd_line::parser& parser) -> bool
{
    if(!engine::create(ctx, parser))
    {
        return false;
    }

    fs::path binary_path = fs::resolve_protocol("binary:/");
    fs::path editor_data = binary_path / "data" / "editor";
    fs::add_path_protocol("editor", editor_data);

    ctx.add<ui_events>();
    ctx.add<project_manager>();
    ctx.add<imgui_interface>(ctx);
    ctx.add<hub>(ctx);
    ctx.add<editing_manager>();
    ctx.add<picking_manager>();
    ctx.add<thumbnail_manager>();

    return true;
}

auto editor::init(const cmd_line::parser& parser) -> bool
{
    if(!engine::init(parser))
    {
        return false;
    }

    auto& ctx = engine::context();

    if(!ctx.get<project_manager>().init(ctx))
    {
        return false;
    }

    if(!ctx.get<imgui_interface>().init(ctx))
    {
        return false;
    }

    if(!ctx.get<hub>().init(ctx))
    {
        return false;
    }

    if(!ctx.get<editing_manager>().init(ctx))
    {
        return false;
    }

    if(!ctx.get<picking_manager>().init(ctx))
    {
        return false;
    }

    if(!ctx.get<thumbnail_manager>().init(ctx))
    {
        return false;
    }

    auto& rend = ctx.get<renderer>();
    auto& win = rend.get_main_window();

    win->get_window().set_title(fmt::format("Ace Editor <{}>", gfx::get_renderer_name(gfx::get_renderer_type())));
    win->get_window().maximize();
    return true;
}

auto editor::deinit() -> bool
{
    auto& ctx = engine::context();

    if(!ctx.get<thumbnail_manager>().deinit(ctx))
    {
        return false;
    }

    if(!ctx.get<picking_manager>().deinit(ctx))
    {
        return false;
    }

    if(!ctx.get<editing_manager>().deinit(ctx))
    {
        return false;
    }

    if(!ctx.get<hub>().deinit(ctx))
    {
        return false;
    }

    if(!ctx.get<imgui_interface>().deinit(ctx))
    {
        return false;
    }

    if(!ctx.get<project_manager>().deinit(ctx))
    {
        return false;
    }

    return engine::deinit();
}

auto editor::destroy() -> bool
{
    auto& ctx = engine::context();

    ctx.remove<thumbnail_manager>();
    ctx.remove<picking_manager>();
    ctx.remove<editing_manager>();

    ctx.remove<hub>();
    ctx.remove<imgui_interface>();

    ctx.remove<project_manager>();

    ctx.remove<ui_events>();

    return engine::destroy();
}

auto editor::process() -> bool
{
    if(!engine::process())
    {
        return false;
    }

    return true;
}
} // namespace ace
