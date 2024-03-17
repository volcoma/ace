#pragma once

#include <edyn/edyn.hpp>
#include <entt/entity/handle.hpp>

namespace edyn
{
void update_rigidbody_mass(entt::entity entity, entt::registry& registry, const rigidbody_def& def);
void update_rigidbody_inertia(entt::entity entity, entt::registry& registry, const rigidbody_def& def);
void update_rigidbody_shape(entt::entity entity, entt::registry& registry, const rigidbody_def& def);
void update_rigidbody_gravity(entt::entity entity, entt::registry& registry, const rigidbody_def& def);
void update_rigidbody_material(entt::entity entity, entt::registry& registry, const rigidbody_def& def);
void update_rigidbody_kind(entt::entity entity, entt::registry& registry, const rigidbody_def& def);

struct rigidbody_owner
{
    entt::handle owner;
};

struct rigidbody
{
    entt::handle internal;
    rigidbody_def def;
};

} // namespace edyn
