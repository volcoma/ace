#include "editor.h"

#include <engine/engine.h>
#include <engine/events.h>
#include <engine/rendering/renderer.h>
#include <engine/assets/asset_watcher.h>

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
    ctx.add<asset_watcher>();

    return true;
}

auto editor::init(const cmd_line::parser& parser) -> bool
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

    if(!ctx.get<asset_watcher>().init(ctx))
    {
        return false;
    }

    if(!engine::init_systems(parser))
    {
        return false;
    }

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

    return true;
}


auto editor::init_window(rtti::context& ctx) -> bool
{
    auto title = fmt::format("Ace Editor <{}>", gfx::get_renderer_name(gfx::get_renderer_type()));
    uint32_t flags = os::window::resizable | os::window::maximized;
    auto primary_display = os::display::get_primary_display_index();

    auto& rend = ctx.get<renderer>();
    rend.create_window_for_display(primary_display, title, flags);
    return true;
}


auto editor::deinit() -> bool
{
    auto& ctx = engine::context();


    if(!ctx.get<asset_watcher>().deinit(ctx))
    {
        return false;
    }

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

    ctx.remove<asset_watcher>();
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
