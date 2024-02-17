#include "box_collider_component.h"
#include "logging/logging.h"
#include "physics/rigidbody_ex.h"
#include <edyn/edyn.hpp>

#include <cstdint>

namespace ace
{

namespace
{
auto is_simulation_active(entt::handle owner) -> bool
{
    return owner.registry()->ctx().contains<edyn::settings>();
}

} // namespace

void box_collider_component::recreate_phyisics_shape()
{
    auto owner = get_owner();
    auto& registry = *owner.registry();
    if(!is_simulation_active(owner))
    {
        return;
    }

    auto body = edyn::try_get_rigidbody(owner);
    if(!body)
    {
        return;
    }
    auto extends = get_extends();
    auto& def = body->def;
    def.shape = edyn::box_shape{extends.x * 0.5f, extends.y * 0.5f, extends.z * 0.5f};

    auto entity = body->entity;
    update_rigidbody_shape(entity.entity(), *entity.registry(), def);
}

void box_collider_component::destroy_phyisics_shape()
{
    auto body = edyn::try_get_rigidbody(get_owner());
    if(body)
    {
        auto& def = body->def;
        def.shape.reset();

        auto entity = body->entity;
        update_rigidbody_shape(entity.entity(), *entity.registry(), def);
    }
}

void box_collider_component::on_create_component(entt::registry& r, const entt::entity e)
{
    entt::handle entity(r, e);

    auto& component = entity.get<box_collider_component>();
    component.set_owner(entity);

    if(is_simulation_active(entity))
    {
        component.on_phyiscs_simulation_begin();
    }
}

void box_collider_component::on_destroy_component(entt::registry& r, const entt::entity e)
{
    entt::handle entity(r, e);
    auto& component = entity.get<box_collider_component>();

    if(is_simulation_active(entity))
    {
        component.on_phyiscs_simulation_end();
    }
}

void box_collider_component::on_change_extends()
{
    recreate_phyisics_shape();
}

void box_collider_component::set_extends(const math::vec3& extends)
{
    if(math::all(math::epsilonEqual(extends_, extends, math::epsilon<float>())))
    {
        return;
    }

    extends_ = extends;

    on_change_extends();
}

auto box_collider_component::get_extends() const noexcept -> const math::vec3&
{
    return extends_;
}

void box_collider_component::on_phyiscs_simulation_begin()
{
    edyn::add_ref_rigidbody(get_owner());
    recreate_phyisics_shape();
}

void box_collider_component::on_phyiscs_simulation_end()
{
    destroy_phyisics_shape();
    edyn::dec_ref_rigidbody(get_owner());
}

} // namespace ace
