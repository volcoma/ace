#include "ecs.h"
#include "components/transform_component.h"
#include "components/id_component.h"

#include "systems/camera_system.h"

namespace ace
{

namespace
{
void setup_transform_component(entt::registry& r, const entt::entity e)
{
    entt::handle entity(r, e);

    auto& component = entity.get<transform_component>();
    component.set_owner(entity);
}
}

ecs::ecs()
{
    registry.on_construct<transform_component>().connect<&setup_transform_component>();
}

auto ecs::init(rtti::context& ctx) -> bool
{
    ctx.add<camera_system>().init(ctx);

    return true;
}

auto ecs::deinit(rtti::context& ctx) -> bool
{
    ctx.remove<camera_system>();

    return true;
}

void ecs::close_project()
{
    registry.clear();
}

auto ecs::create_entity(entt::handle parent) -> entt::handle
{
    entt::handle ent(registry, registry.create());
    ent.emplace<id_component>("Entity");

    auto& transform = ent.emplace<transform_component>();
    if(parent)
    {
        transform.set_parent(parent);
    }

    return ent;
}

auto ecs::create_test_scene() -> entt::handle
{
    auto root = create_entity();
    auto e1 = create_entity();
    e1.get<transform_component>().set_parent(root);

    auto e2 = create_entity();
    e2.get<transform_component>().set_parent(e1);

    return root;
}

} // namespace ace
