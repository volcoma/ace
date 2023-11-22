#include "editor.h"

#include <engine/engine.h>
#include <engine/events.h>
#include <rttr/registration>

#include "events.h"
#include "hub/hub.h"
#include "imgui/imgui_interface.h"
#include "system/project_manager.h"
#include "editing/editing_manager.h"
#include "editing/thumbnail_manager.h"
#include "assets/asset_compiler.h"

#include <iostream>

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

bool editor::create(rtti::context& ctx, cmd_line::parser& parser)
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
    ctx.add<thumbnail_manager>();

    return true;
}

bool editor::init(rtti::context& ctx, const cmd_line::parser& parser)
{
    if(!engine::init(ctx, parser))
    {
        return false;
    }

    ctx.get<thumbnail_manager>().init(ctx);
    ctx.get<project_manager>().init(ctx);
    ctx.get<imgui_interface>().init(ctx);
    ctx.get<hub>().init(ctx);

    return true;
}

bool editor::deinit(rtti::context& ctx)
{
    ctx.get<hub>().deinit(ctx);

    return engine::deinit(ctx);
}


bool editor::destroy(rtti::context& ctx)
{
    ctx.remove<thumbnail_manager>();
    ctx.remove<editing_manager>();

    ctx.remove<hub>();
    ctx.remove<project_manager>();

    ctx.remove<ui_events>();
    ctx.remove<imgui_interface>();

    return engine::destroy(ctx);
}

bool editor::process(rtti::context& ctx)
{
    if(!engine::process(ctx))
    {
        return false;
    }

    return true;
}
} // namespace ace
