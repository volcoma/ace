#pragma once

#include <edyn/dynamics/moment_of_inertia.hpp>
#include <edyn/edyn.hpp>
#include <edyn/util/aabb_util.hpp>
#include <edyn/util/rigidbody.hpp>

#include <entt/entity/handle.hpp>

namespace edyn
{

struct rigidbody_shared
{
    static void on_create_component(entt::registry& r, const entt::entity e);
    static void on_destroy_component(entt::registry& r, const entt::entity e);

    void add_ref()
    {
        ref_count++;
    }
    auto dec_ref() -> bool
    {
        ref_count--;
        return ref_count <= 0;
    }
    entt::handle entity{};

    edyn::rigidbody_def def{};

private:
    int ref_count{};
};

auto try_get_rigidbody(entt::handle owner) -> rigidbody_shared*;
auto try_get_rigidbody(entt::const_handle owner) -> const rigidbody_shared*;
auto add_ref_rigidbody(entt::handle owner) -> rigidbody_shared&;
void recreate_ref_rigidbody(rigidbody_shared& body);

void dec_ref_rigidbody(entt::handle owner);
void remove_rigidbody_shape(entt::entity entity, entt::registry& registry);
void update_rigidbody_mass(entt::entity entity, entt::registry& registry, const rigidbody_def& def);
void update_rigidbody_shape(entt::entity entity, entt::registry& registry, const rigidbody_def& def);
void update_rigidbody_gravity(entt::entity entity, entt::registry& registry, const rigidbody_def& def);

} // namespace edyn
