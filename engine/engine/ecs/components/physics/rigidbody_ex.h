#pragma once

#include <edyn/dynamics/moment_of_inertia.hpp>
#include <edyn/edyn.hpp>
#include <edyn/util/aabb_util.hpp>
#include <edyn/util/rigidbody.hpp>

#include <entt/entity/handle.hpp>

namespace edyn
{
void remove_rigidbody_shape(entt::entity entity, entt::registry& registry);
void update_rigidbody_mass(entt::entity entity, entt::registry& registry, const rigidbody_def& def);
void update_rigidbody_shape(entt::entity entity, entt::registry& registry, const rigidbody_def& def);
void update_rigidbody_gravity(entt::entity entity, entt::registry& registry, const rigidbody_def& def);
void update_rigidbody_material(entt::entity entity, entt::registry& registry, const rigidbody_def& def);


struct physics_body
{
    entt::handle owner;
};

} // namespace edyn
