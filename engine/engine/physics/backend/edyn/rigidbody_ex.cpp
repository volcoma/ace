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

        wake_up_entity(registry, entity);
    }
    else
    {
        registry.emplace_or_replace<mass>(entity, EDYN_SCALAR_MAX);
        registry.emplace_or_replace<mass_inv>(entity, scalar(0));
    }
}

void update_rigidbody_shape(entt::entity entity, entt::registry& registry, const rigidbody_def& def)
{
    rigidbody_set_shape(registry, entity, def.shape);
    wake_up_entity(registry, entity);
}

void update_rigidbody_kind(entt::entity entity, entt::registry& registry, const rigidbody_def& def)
{
    auto& kind = def.kind;
    switch (kind) {
        case rigidbody_kind::rb_dynamic:
            registry.remove<static_tag, kinematic_tag>(entity);
            registry.emplace<dynamic_tag>(entity);
            registry.emplace<procedural_tag>(entity);
            break;
        case rigidbody_kind::rb_kinematic:
            registry.remove<dynamic_tag, static_tag, procedural_tag>(entity);
            registry.emplace<kinematic_tag>(entity);
            break;
        case rigidbody_kind::rb_static:
            registry.remove<dynamic_tag, kinematic_tag, procedural_tag>(entity);
            registry.emplace<static_tag>(entity);
            break;
    }

    if (kind == rigidbody_kind::rb_dynamic) {
        auto &mass = registry.get<edyn::mass>(entity);
        EDYN_ASSERT(mass > EDYN_EPSILON && mass < large_scalar, "Dynamic rigid body must have non-zero mass.");
        auto &inertia = registry.get<edyn::inertia>(entity);
        EDYN_ASSERT(inertia != matrix3x3_zero, "Dynamic rigid body must have non-zero inertia.");
    }

    rigidbody_set_kind(registry, entity, def.kind);
    wake_up_entity(registry, entity);
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

    if(def.kind == rigidbody_kind::rb_dynamic)
    {
        wake_up_entity(registry, entity);
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

    if(def.kind == rigidbody_kind::rb_dynamic)
    {
        wake_up_entity(registry, entity);
    }
}

} // namespace edyn
