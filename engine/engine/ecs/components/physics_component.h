#pragma once

#include "basic_component.h"
#include <entt/entity/fwd.hpp>
#include <edyn/util/rigidbody.hpp>
#include <bitset>
#include <math/math.h>

namespace ace
{

struct physics_box_shape
{
    math::vec3 center{};
    math::vec3 extends{1.0f, 1.0f, 1.0f};
};

struct physics_sphere_shape
{
    math::vec3 center{};
    float radius;
};

struct physics_compound_shape
{
    // Variant with types of shapes a compound is able to hold.
    using shape_t = std::variant<
        physics_box_shape,
        physics_sphere_shape
//        cylinder_shape,
//        capsule_shape,
//        box_shape,
//        polyhedron_shape
    >;


    shape_t shape;
};

class physics_component : public component_crtp<physics_component, owned_component>
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

    auto get_shapes_count() const -> size_t;
    auto get_shape_by_index(size_t index) const -> const physics_compound_shape&;
    void set_shape_by_index(size_t index, const physics_compound_shape& shape);
    auto get_shape() const -> const std::vector<physics_compound_shape>&;
    void set_shape(const std::vector<physics_compound_shape>& shape);

    void on_phyiscs_simulation_begin();
    void on_phyiscs_simulation_end();

    void on_start_load();
    void on_end_load();

    void sync_transforms(const math::transform& transform);
    auto sync_transforms(math::transform& transform) -> bool;

    auto get_simulation_entity() const -> entt::const_handle;

    auto get_def() const -> const edyn::rigidbody_def&;
private:
    auto is_simulation_running() const -> bool;
    void update_def_mass(edyn::rigidbody_def& def);
    void update_def_gravity(edyn::rigidbody_def& def);
    void update_def_kind(edyn::rigidbody_def& def);
    void update_def_shape(edyn::rigidbody_def& def);

    void on_change_gravity();
    void on_change_mass();
    void on_change_kind();
    void on_change_shape();
    void recreate_phyisics_body();
    void recreate_phyisics_entity();

    bool is_kinematic_{};
    bool is_using_gravity_{};
    float mass_{1};

    std::vector<physics_compound_shape> compound_shape_{};

    entt::handle physics_entity_{};
    edyn::rigidbody_def def_{};


    bool is_loading{};
    std::bitset<32> dirty_;
};
} // namespace ace
