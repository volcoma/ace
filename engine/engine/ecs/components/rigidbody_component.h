#pragma once

#include "basic_component.h"
#include <math/math.h>

namespace ace
{

enum class rigidbody_kind : uint8_t
{
    // A rigid body with non-zero and finite mass that reacts to forces and
    // impulses and can be affected by constraints.
    rb_dynamic,

    // A rigid body that is not affected by others and can be moved directly.
    rb_kinematic,

    // A rigid body that is not affected by others and never changes.
    rb_static
};

class rigidbody_component : public component_crtp<rigidbody_component, owned_component>
{
public:
    static void on_create_component(entt::registry& r, const entt::entity e);
    static void on_destroy_component(entt::registry& r, const entt::entity e);

    void set_is_using_gravity(bool use_gravity);
    auto get_is_using_gravity() const noexcept -> bool;

    void on_phyiscs_simulation_begin();
    void on_phyiscs_simulation_end();

private:
    void on_change_gravity();


    rigidbody_kind kind_ {rigidbody_kind::rb_dynamic};
    float mass_{1};
    bool use_gravity_{};
};
} // namespace ace
