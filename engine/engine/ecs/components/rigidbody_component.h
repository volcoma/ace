#pragma once

#include "basic_component.h"
#include <entt/entity/fwd.hpp>
#include <edyn/util/rigidbody.hpp>
#include <bitset>
#include <math/math.h>

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

    void set_mass(float mass);
    auto get_mass() const noexcept -> float;

    auto is_dirty(uint8_t id) const noexcept -> bool;
    void set_dirty(uint8_t id, bool dirty) noexcept;

    auto get_internal_phyisics_entity() const -> entt::const_handle;
    auto get_internal_phyisics_entity() -> entt::handle;


    void on_phyiscs_simulation_begin();
    void on_phyiscs_simulation_end();

    void on_start_load();
    void on_end_load();
private:
    void update_def(edyn::rigidbody_def& def);
    void on_change_gravity();
    void on_change_mass();
    void on_change_kind();
    void recreate_phyisics_body();
    auto is_simulation_running() const -> bool;

    bool is_kinematic_{};
    bool is_using_gravity_{};
    float mass_{1};

    bool is_loading{};
    std::bitset<32> dirty_;
};
} // namespace ace
