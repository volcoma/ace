#include "editing_manager.h"
#include <engine/ecs/ecs.h>
#include <engine/events.h>
#include <imgui_widgets/gizmo.h>

namespace ace
{
namespace
{

auto get_cached_scene() -> std::unique_ptr<scene>&
{
    static std::unique_ptr<scene> scn;
    return scn;
}
} // namespace

auto editing_manager::init(rtti::context& ctx) -> bool
{
    auto& ev = ctx.get<events>();

    ev.on_play_begin.connect(sentinel_, this, &editing_manager::on_play_begin);

    ev.on_play_end.connect(sentinel_, this, &editing_manager::on_play_end);
    return true;
}

auto editing_manager::deinit(rtti::context& ctx) -> bool
{
    auto& cached_scene = get_cached_scene();
    if(cached_scene)
    {
        cached_scene->unload();
        cached_scene.reset();
    }

    unselect();
    unfocus();
    return true;
}

void editing_manager::on_play_begin(rtti::context& ctx)
{
    auto& ec = ctx.get<ecs>();
    auto& scn = ec.get_scene();

    auto& cached_scene = get_cached_scene();
    cached_scene = std::make_unique<scene>();
    scene::clone_scene(scn, *cached_scene);

    unselect();
    unfocus();
}

void editing_manager::on_play_end(rtti::context& ctx)
{
    auto& ec = ctx.get<ecs>();
    auto& scn = ec.get_scene();

    auto& cached_scene = get_cached_scene();
    scene::clone_scene(*cached_scene, scn);

    cached_scene->unload();
    cached_scene.reset();

    unselect();
    unfocus();
}

void editing_manager::select(rttr::variant object)
{
    selection_data.object = object;
}

void editing_manager::focus(rttr::variant object)
{
    focused_data.object = object;
}

void editing_manager::unselect()
{
    selection_data = {};
    ImGuizmo::Enable(false);
    ImGuizmo::Enable(true);
}

void editing_manager::unfocus()
{
    focused_data = {};
}

void editing_manager::close_project()
{
    unselect();
    unfocus();
}
} // namespace ace
