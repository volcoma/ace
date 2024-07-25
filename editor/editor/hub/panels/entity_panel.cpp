#include "entity_panel.h"
#include <editor/editing/editing_manager.h>
#include <engine/defaults/defaults.h>
#include <engine/engine.h>
#include "panel.h"

namespace ace
{

entity_panel::entity_panel(imgui_panels* parent) : parent_(parent)
{
}
void entity_panel::on_frame_ui_render()
{
    execute_actions();
}

void entity_panel::duplicate_entity(entt::handle entity)
{
    add_action(
        [entity]() mutable
        {
            if(!entity.valid())
            {
                return;
            }

            auto& ctx = engine::context();
            auto& ec = ctx.get<ecs>();
            auto& em = ctx.get<editing_manager>();

            auto object = ec.get_scene().clone_entity(entity);
            em.select(object);
        });
}

void entity_panel::focus_entity(entt::handle camera, entt::handle entity)
{
    add_action(
        [camera, entity]() mutable
        {
            if(!entity.valid())
            {
                return;
            }
            defaults::focus_camera_on_entity(camera, entity);
        });
}

void entity_panel::delete_entity(entt::handle entity)
{
    add_action(
        [entity]() mutable
        {
            if(!entity.valid())
            {
                return;
            }
            entity.destroy();
        });
}

void entity_panel::add_action(const action_t& action)
{
    actions.emplace_back(action);
}

void entity_panel::execute_actions()
{
    for(const auto& action : actions)
    {
        action();
    }
    actions.clear();
}

} // namespace ace
