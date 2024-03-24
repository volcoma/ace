#include "physics_component.h"
#include <engine/ecs/components/transform_component.h>
#include <engine/physics/backend/edyn/rigidbody_ex.h>

#include <cstdint>

namespace ace
{

void physics_component::on_create_component(entt::registry& r, const entt::entity e)
{
    entt::handle entity(r, e);

    auto& component = entity.get<physics_component>();
    component.set_owner(entity);
    component.dirty_.set();
    component.dirty_properties_.set();
}

void physics_component::on_destroy_component(entt::registry& r, const entt::entity e)
{
}

void physics_component::set_is_kinematic(bool kinematic)
{
    if(is_kinematic_ == kinematic)
    {
        return;
    }

    is_kinematic_ = kinematic;

    on_change_kind();
}
auto physics_component::is_kinematic() const noexcept -> bool
{
    return is_kinematic_;
}

void physics_component::on_change_kind()
{
    dirty_.set();
    set_property_dirty(physics_property::kind, true);
}

void physics_component::set_is_using_gravity(bool use_gravity)
{
    if(is_using_gravity_ == use_gravity)
    {
        return;
    }

    is_using_gravity_ = use_gravity;

    on_change_gravity();
}

auto physics_component::is_using_gravity() const noexcept -> bool
{
    return is_using_gravity_;
}

void physics_component::on_change_gravity()
{
    dirty_.set();
    set_property_dirty(physics_property::gravity, true);
}

void physics_component::set_mass(float mass)
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
auto physics_component::get_mass() const noexcept -> float
{
    return mass_;
}

void physics_component::on_change_mass()
{
    dirty_.set();
    set_property_dirty(physics_property::mass, true);
}

void physics_component::set_is_sensor(bool sensor)
{
    if(is_sensor_ == sensor)
    {
        return;
    }

    is_sensor_ = sensor;

    on_change_sensor();
}
auto physics_component::is_sensor() const noexcept -> bool
{
    return is_sensor_;
}

void physics_component::on_change_sensor()
{
    dirty_.set();
    set_property_dirty(physics_property::sensor, true);
}

auto physics_component::is_dirty(uint8_t id) const noexcept -> bool
{
    return dirty_[id];
}

void physics_component::set_dirty(uint8_t id, bool dirty) noexcept
{
    dirty_.set(id, dirty);

    if(!dirty)
    {
        dirty_properties_ = {};
    }
}

auto physics_component::is_property_dirty(physics_property prop) const noexcept -> bool
{
    return dirty_properties_[static_cast<std::underlying_type_t<physics_property>>(prop)];
}

auto physics_component::are_any_properties_dirty() const noexcept -> bool
{
    return dirty_properties_.any();
}

auto physics_component::are_all_properties_dirty() const noexcept -> bool
{
    return dirty_properties_.all();
}

void physics_component::set_property_dirty(physics_property prop, bool dirty) noexcept
{
    dirty_properties_[static_cast<std::underlying_type_t<physics_property>>(prop)] = dirty;
}

auto physics_component::get_shapes_count() const -> size_t
{
    return compound_shape_.size();
}
auto physics_component::get_shape_by_index(size_t index) const -> const physics_compound_shape&
{
    return compound_shape_.at(index);
}
void physics_component::set_shape_by_index(size_t index, const physics_compound_shape& shape)
{
    compound_shape_.at(index) = shape;
}

auto physics_component::get_shapes() const -> const std::vector<physics_compound_shape>&
{
    return compound_shape_;
}
void physics_component::set_shapes(const std::vector<physics_compound_shape>& shape)
{
    compound_shape_ = shape;

    on_change_shape();
}

void physics_component::on_change_shape()
{
    dirty_.set();
    set_property_dirty(physics_property::shape, true);
}

auto physics_component::get_material() const -> const asset_handle<physics_material>&
{
    return material_;
}
void physics_component::set_material(const asset_handle<physics_material>& material)
{
    if(material_ == material)
    {
        return;
    }
    material_ = material;

    on_change_material();
}

void physics_component::on_change_material()
{
    dirty_.set();
    set_property_dirty(physics_property::material, true);
}

void physics_component::apply_impulse(const math::vec3& impulse)
{
    auto owner = get_owner();
    auto& registry = *owner.registry();
    auto& emitter = registry.ctx().get<physics_component_emitter>();

    emitter.apply_impulse.publish(*this, impulse);
}

void physics_component::apply_torque_impulse(const math::vec3& torque_impulse)
{
    auto owner = get_owner();
    auto& registry = *owner.registry();
    auto& emitter = registry.ctx().get<physics_component_emitter>();

    emitter.apply_torque_impulse.publish(*this, torque_impulse);
}

void physics_component::clear_kinematic_velocities()
{
    auto owner = get_owner();
    auto& registry = *owner.registry();
    auto& emitter = registry.ctx().get<physics_component_emitter>();

    emitter.clear_kinematic_velocities.publish(*this);
}

} // namespace ace
