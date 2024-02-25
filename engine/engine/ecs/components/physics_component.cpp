#include "physics_component.h"
#include "entt/entity/fwd.hpp"
#include "transform_component.h"

#include "edyn/math/quaternion.hpp"
#include "edyn/shapes/compound_shape.hpp"
#include "edyn/util/rigidbody.hpp"
#include "physics/rigidbody_ex.h"
#include <cstdint>

namespace ace
{

namespace
{
auto max3(const math::vec3& v) -> float
{
    return math::max(math::max(v.x, v.y), v.z);
}
}

void physics_component::update_def_mass(edyn::rigidbody_def& def)
{
    def.mass = mass_;
}

void physics_component::update_def_gravity(edyn::rigidbody_def& def)
{
    auto registry = get_owner().registry();
    def.gravity = is_using_gravity() ? edyn::get_gravity(*registry) : edyn::vector3_zero;
}

void physics_component::update_def_kind(edyn::rigidbody_def& def)
{
    def.kind = is_kinematic() ? edyn::rigidbody_kind::rb_kinematic : edyn::rigidbody_kind::rb_dynamic;
}

void physics_component::update_def_shape(edyn::rigidbody_def& def)
{
    if(compound_shape_.empty())
    {
        def.shape = {};
        def.inertia = edyn::matrix3x3_identity;
    }
    else
    {
        edyn::compound_shape cp;

        auto& transform = get_owner().get<transform_component>();
        const auto& scale = transform.get_scale_global();

        for(const auto& s : compound_shape_)
        {
            if(std::holds_alternative<physics_box_shape>(s.shape))
            {
                auto& shape = std::get<physics_box_shape>(s.shape);
                auto extends = shape.extends * scale;

                edyn::box_shape box_shape{extends.x * 0.5f, extends.y * 0.5f, extends.z * 0.5f};
                edyn::vector3 center{shape.center.x, shape.center.y, shape.center.z};
                cp.add_shape(box_shape, center, edyn::quaternion_identity);
            }
            else if(std::holds_alternative<physics_sphere_shape>(s.shape))
            {
                auto& shape = std::get<physics_sphere_shape>(s.shape);
                auto radius = shape.radius * max3(scale);

                edyn::sphere_shape sphere_shape{radius};
                edyn::vector3 center{shape.center.x, shape.center.y, shape.center.z};
                cp.add_shape(sphere_shape, center, edyn::quaternion_identity);
            }
            else if(std::holds_alternative<physics_capsule_shape>(s.shape))
            {
                auto& shape = std::get<physics_capsule_shape>(s.shape);
                auto radius = shape.radius * max3(scale);
                auto half_length = shape.length * 0.5f * max3(scale);

                edyn::capsule_shape capsule_shape{radius, half_length, edyn::coordinate_axis::y};
                edyn::vector3 center{shape.center.x, shape.center.y, shape.center.z};
                cp.add_shape(capsule_shape, center, edyn::quaternion_identity);
            }        
            else if(std::holds_alternative<physics_cylinder_shape>(s.shape))
            {
                auto& shape = std::get<physics_cylinder_shape>(s.shape);
                auto radius = shape.radius * max3(scale);
                auto half_length = shape.length * 0.5f * max3(scale);

                edyn::cylinder_shape cylinder_shape{radius, half_length, edyn::coordinate_axis::y};
                edyn::vector3 center{shape.center.x, shape.center.y, shape.center.z};
                cp.add_shape(cylinder_shape, center, edyn::quaternion_identity);
            }
        }
        cp.finish();
        def.shape = cp;
        def.inertia.reset();
    }
}

void physics_component::recreate_phyisics_body()
{
    dirty_.set();

    update_def_mass(def_);
    update_def_kind(def_);
    update_def_shape(def_);

    if(is_simulation_running())
    {
        update_def_gravity(def_);

        recreate_phyisics_entity();
        edyn::make_rigidbody(physics_entity_.entity(), *physics_entity_.registry(), def_);
    }

}

void physics_component::recreate_phyisics_entity()
{
    if(physics_entity_)
    {
        physics_entity_.destroy();
    }

    auto registry = get_owner().registry();
    physics_entity_ = entt::handle(*registry, registry->create());
}

void physics_component::on_create_component(entt::registry& r, const entt::entity e)
{
    entt::handle entity(r, e);

    auto& component = entity.get<physics_component>();
    component.set_owner(entity);
    component.physics_entity_ = {};

    if(component.is_simulation_running())
    {
        component.on_phyiscs_simulation_begin();
    }
}

void physics_component::on_destroy_component(entt::registry& r, const entt::entity e)
{
    entt::handle entity(r, e);
    auto& component = entity.get<physics_component>();

    if(component.is_simulation_running())
    {
        component.on_phyiscs_simulation_end();
    }
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

    update_def_kind(def_);

    if(is_simulation_running())
    {
        recreate_phyisics_entity();
        edyn::make_rigidbody(physics_entity_.entity(), *physics_entity_.registry(), def_);
    }
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

    if(is_simulation_running())
    {
        update_def_gravity(def_);

        update_rigidbody_gravity(physics_entity_.entity(), *physics_entity_.registry(), def_);
    }

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

    update_def_mass(def_);

    if(is_simulation_running())
    {
        update_rigidbody_mass(physics_entity_.entity(), *physics_entity_.registry(), def_);
    }

}

void physics_component::on_phyiscs_simulation_begin()
{
    recreate_phyisics_body();
}

void physics_component::on_phyiscs_simulation_end()
{
    if(physics_entity_)
    {
        physics_entity_.destroy();
    }
}

void physics_component::on_start_load()
{
    is_loading = true;
}

void physics_component::on_end_load()
{
    is_loading = false;
    if(dirty_.any() && is_simulation_running())
    {
        recreate_phyisics_body();
    }
}

void physics_component::sync_transforms(const math::transform& transform)
{
    if(!physics_entity_)
    {
        return;
    }
    auto [epos, eorientation] = physics_entity_.get<edyn::position, edyn::orientation>();

    const auto& p = transform.get_position();
    epos.x = p.x;
    epos.y = p.y;
    epos.z = p.z;
    physics_entity_.patch<edyn::position>();

    const auto& q = transform.get_rotation();
    eorientation.x = q.x;
    eorientation.y = q.y;
    eorientation.z = q.z;
    eorientation.w = q.w;
    physics_entity_.patch<edyn::orientation>();

    edyn::wake_up_entity(*physics_entity_.registry(), physics_entity_.entity());
}

auto physics_component::sync_transforms(math::transform& transform) -> bool
{
    if(!physics_entity_)
    {
        return false;
    }

    auto [epos, eorientation] = physics_entity_.try_get<edyn::present_position, edyn::present_orientation>();

    if(epos)
    {
        math::vec3 p;
        p.x = epos->x;
        p.y = epos->y;
        p.z = epos->z;
        transform.set_position(p);
    }

    if(eorientation)
    {
        math::quat q;
        q.x = eorientation->x;
        q.y = eorientation->y;
        q.z = eorientation->z;
        q.w = eorientation->w;
        transform.set_rotation(q);
    }

    return epos || eorientation;
}

auto physics_component::is_dirty(uint8_t id) const noexcept -> bool
{
    return dirty_[id];
}

void physics_component::set_dirty(uint8_t id, bool dirty) noexcept
{
    dirty_.set(id, dirty);
}

auto physics_component::is_simulation_running() const -> bool
{
    return !is_loading && get_owner().registry()->ctx().contains<edyn::settings>();
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
    recreate_phyisics_body();
}

auto physics_component::get_simulation_entity() const -> entt::const_handle
{
    return physics_entity_;
}


auto physics_component::get_def() const -> const edyn::rigidbody_def&
{
    return def_;
}


} // namespace ace
