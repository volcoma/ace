#pragma once

#include <engine/ecs/components/basic_component.h>
#include <engine/physics/physics_material.h>
#include <hpp/variant.hpp>
#include <math/math.h>

#include <bitset>
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
    float radius{0.5f};
};

struct physics_capsule_shape
{
    math::vec3 center{};
    float radius{0.5f};
    float length{1.0f};
    // coordinate_axis axis {coordinate_axis::x};
};

struct physics_cylinder_shape
{
    math::vec3 center{};
    float radius{0.5f};
    float length{1.0f};
    // coordinate_axis axis {coordinate_axis::x};
};

struct physics_compound_shape
{
    // Variant with types of shapes a compound is able to hold.
    using shape_t = hpp::variant<physics_box_shape, physics_sphere_shape, physics_capsule_shape, physics_cylinder_shape
                                 //        polyhedron_shape
                                 >;

    shape_t shape;
};

enum class physics_property : uint8_t
{
    gravity,
    kind,
    mass,
    material,
    shape,
    sensor,
    count
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

    void set_is_sensor(bool sensor);
    auto is_sensor() const noexcept -> bool;

    auto is_dirty(uint8_t id) const noexcept -> bool;
    void set_dirty(uint8_t id, bool dirty) noexcept;
    auto is_property_dirty(physics_property prop) const noexcept -> bool;
    auto are_any_properties_dirty() const noexcept -> bool;
    auto are_all_properties_dirty() const noexcept -> bool;

    void set_property_dirty(physics_property prop, bool dirty) noexcept;

    auto get_shapes_count() const -> size_t;
    auto get_shape_by_index(size_t index) const -> const physics_compound_shape&;
    void set_shape_by_index(size_t index, const physics_compound_shape& shape);
    auto get_shapes() const -> const std::vector<physics_compound_shape>&;
    void set_shapes(const std::vector<physics_compound_shape>& shape);

    auto get_material() const -> const asset_handle<physics_material>&;
    void set_material(const asset_handle<physics_material>& material);

    void apply_impulse(const math::vec3& impulse);
    void apply_torque_impulse(const math::vec3& torque_impulse);
    void clear_kinematic_velocities();

private:
    void on_change_gravity();
    void on_change_mass();
    void on_change_kind();
    void on_change_shape();
    void on_change_material();
    void on_change_sensor();

    bool is_kinematic_{};
    bool is_using_gravity_{};
    bool is_sensor_{};
    float mass_{1};

    asset_handle<physics_material> material_{};
    std::vector<physics_compound_shape> compound_shape_{};

    std::bitset<static_cast<std::underlying_type_t<physics_property>>(physics_property::count)> dirty_properties_;
    std::bitset<8> dirty_;
};

struct physics_component_emitter
{
#define DEFINE_SIGNAL(type, name)                                                                                      \
    auto on_##name()                                                                                                   \
    {                                                                                                                  \
        return entt::sink{name};                                                                                       \
    }                                                                                                                  \
    type name

    DEFINE_SIGNAL(entt::sigh<void(physics_component&, const math::vec3&)>, apply_impulse);
    DEFINE_SIGNAL(entt::sigh<void(physics_component&, const math::vec3&)>, apply_torque_impulse);
    DEFINE_SIGNAL(entt::sigh<void(physics_component&)>, clear_kinematic_velocities);
    DEFINE_SIGNAL(entt::sigh<void(entt::registry& r, const entt::entity e)>, create_component);
    DEFINE_SIGNAL(entt::sigh<void(entt::registry& r, const entt::entity e)>, destroy_component);

};

} // namespace ace
