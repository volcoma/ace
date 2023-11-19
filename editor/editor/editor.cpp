#include "editor.h"

#include <engine/engine.h>
#include <engine/events.h>
#include <rttr/registration>

#include "events.h"
#include "hub/hub.h"
#include "imgui/imgui_interface.h"
#include "system/project_manager.h"
#include "editing/editing_system.h"
#include "editing/thumbnail_system.h"
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
    ctx.add<editing_system>();
    ctx.add<thumbnail_system>();

    return true;
}

bool editor::init(rtti::context& ctx, const cmd_line::parser& parser)
{
    if(!engine::init(ctx, parser))
    {
        return false;
    }

    ctx.get<thumbnail_system>().init(ctx);
    ctx.get<project_manager>().init(ctx);
    ctx.get<imgui_interface>().init(ctx);
    ctx.get<hub>().init(ctx);

    return true;
}

bool editor::deinit(rtti::context& ctx)
{
    ctx.remove<thumbnail_system>();
    ctx.remove<editing_system>();

    ctx.remove<hub>();
    ctx.remove<project_manager>();

    ctx.remove<ui_events>();
    ctx.remove<imgui_interface>();

    return engine::deinit(ctx);
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
