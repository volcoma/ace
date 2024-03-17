#include "rigidbody_ex.h"
#include <edyn/dynamics/moment_of_inertia.hpp>
#include <edyn/util/aabb_util.hpp>

namespace edyn
{

void update_rigidbody_mass(entt::entity entity, entt::registry& registry, const rigidbody_def& def)
{
    if(def.kind == rigidbody_kind::rb_dynamic)
    {
        EDYN_ASSERT(def.mass > EDYN_EPSILON && def.mass < large_scalar, "Dynamic rigid body must have non-zero mass.");
        registry.emplace_or_replace<mass>(entity, def.mass);
        registry.emplace_or_replace<mass_inv>(entity, scalar(1) / def.mass);
    }
    else
    {
        registry.emplace_or_replace<mass>(entity, EDYN_SCALAR_MAX);
        registry.emplace_or_replace<mass_inv>(entity, scalar(0));
    }
}

void update_rigidbody_inertia(entt::entity entity, entt::registry& registry, const rigidbody_def& def)
{
    if(def.kind == rigidbody_kind::rb_dynamic)
    {
        matrix3x3 inertia;

        if(def.inertia)
        {
            inertia = *def.inertia;
        }
        else
        {
            EDYN_ASSERT(def.shape.has_value(),
                        "A shape must be provided if a pre-calculated inertia hasn't been assigned.");
            inertia = moment_of_inertia(*def.shape, def.mass);

            if(def.center_of_mass)
            {
                // Use parallel-axis theorem to calculate moment of inertia along
                // axes away from the origin.
                inertia = shift_moment_of_inertia(inertia, def.mass, *def.center_of_mass);
            }
        }

        auto I_inv = inverse_matrix_symmetric(inertia);
        registry.emplace_or_replace<edyn::inertia>(entity, inertia);
        registry.emplace_or_replace<inertia_inv>(entity, I_inv);

        auto basis = to_matrix3x3(def.orientation);
        auto I_inv_world = basis * I_inv * transpose(basis);
        registry.emplace_or_replace<inertia_world_inv>(entity, I_inv_world);
    }
    else
    {
        registry.emplace_or_replace<inertia>(entity, matrix3x3_zero);
        registry.emplace_or_replace<inertia_inv>(entity, matrix3x3_zero);
        registry.emplace_or_replace<inertia_world_inv>(entity, matrix3x3_zero);
    }
}

void update_rigidbody_shape(entt::entity entity, entt::registry& registry, const rigidbody_def& def)
{
    rigidbody_set_shape(registry, entity, def.shape);
}

void update_rigidbody_kind(entt::entity entity, entt::registry& registry, const rigidbody_def& def)
{
    rigidbody_set_kind(registry, entity, def.kind);
}

void update_rigidbody_gravity(entt::entity entity, entt::registry& registry, const rigidbody_def& def)
{
    auto g = def.gravity ? *def.gravity : get_gravity(registry);

    if(g != vector3_zero && def.kind == rigidbody_kind::rb_dynamic)
    {
        registry.emplace_or_replace<gravity>(entity, g);
    }
    else
    {
        registry.emplace_or_replace<linvel>(entity, vector3_zero);
        registry.remove<gravity>(entity);
    }
}

void update_rigidbody_material(entt::entity entity, entt::registry& registry, const rigidbody_def& def)
{
    if(def.material)
    {
        registry.emplace_or_replace<material>(entity, *def.material);

        set_rigidbody_friction(registry, entity, def.material->friction);
    }
    else
    {
        registry.remove<material>(entity);
    }
}

} // namespace edyn
