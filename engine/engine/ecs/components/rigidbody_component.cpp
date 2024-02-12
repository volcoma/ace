#include "rigidbody_component.h"
#include <edyn/edyn.hpp>

#include <cstdint>

namespace ace
{

namespace
{
auto is_simulation_active(const entt::registry& registry) -> bool
{
    return registry.ctx().contains<edyn::settings>();
}

} // namespace

void rigidbody_component::on_create_component(entt::registry& r, const entt::entity e)
{
    entt::handle entity(r, e);

    auto& component = entity.get<rigidbody_component>();
    component.set_owner(entity);
    component.on_phyiscs_simulation_begin();
}

void rigidbody_component::on_destroy_component(entt::registry& r, const entt::entity e)
{
    entt::handle entity(r, e);
    auto& component = entity.get<rigidbody_component>();
    component.on_phyiscs_simulation_end();
}

void rigidbody_component::on_change_gravity()
{
    auto entity = get_owner();
    auto& registry = *entity.registry();
    if(!is_simulation_active(registry))
    {
        return;
    }

    if(get_is_using_gravity())
    {
        auto gravity = edyn::get_gravity(registry);

        if(kind_ == rigidbody_kind::rb_dynamic)
        {
            registry.emplace_or_replace<edyn::gravity>(entity, gravity);
            registry.patch<edyn::gravity>(entity);
        }
    }
    else
    {
        registry.replace<edyn::linvel>(entity, edyn::vector3_zero);
        registry.remove<edyn::gravity>(entity);
    }
}

void rigidbody_component::set_is_using_gravity(bool use_gravity)
{
    if(use_gravity_ == use_gravity)
    {
        return;
    }

    use_gravity_ = use_gravity;

    on_change_gravity();
}

auto rigidbody_component::get_is_using_gravity() const noexcept -> bool
{
    return use_gravity_;
}

void rigidbody_component::on_phyiscs_simulation_begin()
{
    auto entity = get_owner();
    auto& registry = *entity.registry();
    if(!is_simulation_active(registry))
    {
        return;
    }

    edyn::rigidbody_def def;
    def.mass = mass_;

    if(!use_gravity_)
    {
        def.gravity = edyn::vector3{0, 0, 0};
    }

//    def.material->friction = 0.8;
//    def.material->restitution = 1.0;
//    def.shape = edyn::box_shape{0.5, 0.5, 0.5};

    edyn::make_rigidbody(entity.entity(), registry, def);
}

void rigidbody_component::on_phyiscs_simulation_end()
{

}

} // namespace ace
