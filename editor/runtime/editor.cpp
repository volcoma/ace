#include "editor.h"

#include <runtime/engine.h>
#include <runtime/events.h>
#include <rttr/registration>

#include "imgui/imgui_interface.h"

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

    ctx.add<imgui_interface>(ctx);

    return true;
}

bool editor::init(rtti::context& ctx, const cmd_line::parser& parser)
{
    if(!engine::init(ctx, parser))
    {
        return false;
    }

    ctx.get<imgui_interface>().init(ctx);

    return true;
}

bool editor::deinit(rtti::context& ctx)
{
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
}
