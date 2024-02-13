#pragma once

#include "basic_component.h"
#include <math/math.h>
#include <bitset>

namespace ace
{

class rigidbody_component : public component_crtp<rigidbody_component, owned_component>
{
public:
    static void on_create_component(entt::registry& r, const entt::entity e);
    static void on_destroy_component(entt::registry& r, const entt::entity e);

    void set_is_using_gravity(bool use_gravity);
    auto is_using_gravity() const noexcept -> bool;

    void set_is_kinematic(bool kinematic);
    auto is_kinematic() const noexcept -> bool;


    void on_phyiscs_simulation_begin();
    void on_phyiscs_simulation_end();

    auto is_dirty(uint8_t id) const noexcept -> bool;
    void set_dirty(uint8_t id, bool dirty) noexcept;
private:
    void on_change_gravity();
    void on_change_kind();

    bool is_kinematic_{};
    bool is_using_gravity_{};
    float mass_{1};

    std::bitset<32> dirty_;
};
} // namespace ace
