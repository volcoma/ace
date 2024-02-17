#include "rigidbody_component.h"
#include "physics/rigidbody_ex.h"
#include <cstdint>

namespace ace
{


void rigidbody_component::update_def(edyn::rigidbody_def& def)
{
    auto registry = get_owner().registry();
    def.mass = mass_;
    def.kind = is_kinematic() ? edyn::rigidbody_kind::rb_kinematic : edyn::rigidbody_kind::rb_dynamic;
    def.gravity = is_using_gravity() ? edyn::get_gravity(*registry) : edyn::vector3_zero;
}

void rigidbody_component::recreate_phyisics_body()
{
    dirty_.set();

    auto owner = get_owner();
    if(!is_simulation_running())
    {
        return;
    }

    auto body = edyn::try_get_rigidbody(owner);

    if(body)
    {
        edyn::recreate_ref_rigidbody(*body);
        update_def(body->def);

        auto entity = body->entity;
        edyn::make_rigidbody(entity.entity(), *entity.registry(), body->def);
    }
}

void rigidbody_component::on_create_component(entt::registry& r, const entt::entity e)
{
    entt::handle entity(r, e);

    auto& component = entity.get<rigidbody_component>();
    component.set_owner(entity);

    if(component.is_simulation_running())
    {
        component.on_phyiscs_simulation_begin();
    }
}

void rigidbody_component::on_destroy_component(entt::registry& r, const entt::entity e)
{
    entt::handle entity(r, e);
    auto& component = entity.get<rigidbody_component>();

    if(component.is_simulation_running())
    {
        component.on_phyiscs_simulation_end();
    }
}

void rigidbody_component::set_is_kinematic(bool kinematic)
{
    if(is_kinematic_ == kinematic)
    {
        return;
    }

    is_kinematic_ = kinematic;

    on_change_kind();
}
auto rigidbody_component::is_kinematic() const noexcept -> bool
{
    return is_kinematic_;
}

void rigidbody_component::on_change_kind()
{
    recreate_phyisics_body();
}

void rigidbody_component::set_is_using_gravity(bool use_gravity)
{
    if(is_using_gravity_ == use_gravity)
    {
        return;
    }

    is_using_gravity_ = use_gravity;

    on_change_gravity();
}

auto rigidbody_component::is_using_gravity() const noexcept -> bool
{
    return is_using_gravity_;
}

void rigidbody_component::on_change_gravity()
{
    dirty_.set();

    auto owner = get_owner();
    auto& registry = *owner.registry();
    if(!is_simulation_running())
    {
        return;
    }

    auto entity = get_internal_phyisics_entity().entity();


    auto body = edyn::try_get_rigidbody(owner);

    if(body)
    {
        update_def(body->def);
        update_rigidbody_gravity(entity, registry, body->def);
    }
}

void rigidbody_component::set_mass(float mass)
{
    if(math::epsilonEqual(mass_, mass, math::epsilon<float>()))
    {
        return;
    }

    if(mass <= EDYN_EPSILON && mass >= edyn::large_scalar)
    {
        return;
    }


    mass_ = mass;

    on_change_mass();
}
auto rigidbody_component::get_mass() const noexcept -> float
{
    return mass_;
}

void rigidbody_component::on_change_mass()
{
    dirty_.set();

    auto owner = get_owner();
    auto& registry = *owner.registry();
    if(!is_simulation_running())
    {
        return;
    }

    auto entity = get_internal_phyisics_entity().entity();


    auto body = edyn::try_get_rigidbody(owner);

    if(body)
    {
        update_def(body->def);
        update_rigidbody_mass(entity, registry, body->def);
    }
}

void rigidbody_component::on_phyiscs_simulation_begin()
{
    edyn::add_ref_rigidbody(get_owner());
    recreate_phyisics_body();
}

void rigidbody_component::on_phyiscs_simulation_end()
{
    edyn::dec_ref_rigidbody(get_owner());
}

void rigidbody_component::on_start_load()
{
    is_loading = true;
}

void rigidbody_component::on_end_load()
{
    is_loading = false;
    if(dirty_.any() && is_simulation_running())
    {
        recreate_phyisics_body();
    }
}

auto rigidbody_component::is_dirty(uint8_t id) const noexcept -> bool
{
    return dirty_[id];
}

void rigidbody_component::set_dirty(uint8_t id, bool dirty) noexcept
{
    dirty_.set(id, dirty);
}

auto rigidbody_component::get_internal_phyisics_entity() const -> entt::const_handle
{
    auto body = edyn::try_get_rigidbody(get_owner());
    if(body)
    {
        return body->entity;
    }
    return {};
}
auto rigidbody_component::get_internal_phyisics_entity() -> entt::handle
{
    auto body = edyn::try_get_rigidbody(get_owner());
    if(body)
    {
        return body->entity;
    }
    return {};
}

auto rigidbody_component::is_simulation_running() const -> bool
{
    return !is_loading && get_owner().registry()->ctx().contains<edyn::settings>();
}


} // namespace ace
