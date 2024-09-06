#pragma once
#include <engine/engine_export.h>

#include <engine/ecs/components/basic_component.h>
#include <engine/physics/physics_material.h>
#include <hpp/variant.hpp>
#include <math/math.h>

#include <bitset>

namespace ace
{

/**
 * @struct physics_box_shape
 * @brief Represents a box shape for physics calculations.
 */
struct physics_box_shape
{
    math::vec3 center{};                  ///< Center of the box.
    math::vec3 extends{1.0f, 1.0f, 1.0f}; ///< Extents of the box.
};

/**
 * @struct physics_sphere_shape
 * @brief Represents a sphere shape for physics calculations.
 */
struct physics_sphere_shape
{
    math::vec3 center{}; ///< Center of the sphere.
    float radius{0.5f};  ///< Radius of the sphere.
};

/**
 * @struct physics_capsule_shape
 * @brief Represents a capsule shape for physics calculations.
 */
struct physics_capsule_shape
{
    math::vec3 center{}; ///< Center of the capsule.
    float radius{0.5f};  ///< Radius of the capsule.
    float length{1.0f};  ///< Length of the capsule.
};

/**
 * @struct physics_cylinder_shape
 * @brief Represents a cylinder shape for physics calculations.
 */
struct physics_cylinder_shape
{
    math::vec3 center{}; ///< Center of the cylinder.
    float radius{0.5f};  ///< Radius of the cylinder.
    float length{1.0f};  ///< Length of the cylinder.
};

/**
 * @struct physics_compound_shape
 * @brief Represents a compound shape that can contain multiple types of shapes.
 */
struct physics_compound_shape
{
    using shape_t =
        hpp::variant<physics_box_shape, physics_sphere_shape, physics_capsule_shape, physics_cylinder_shape>;

    shape_t shape; ///< The shape contained in the compound shape.
};

/**
 * @enum physics_property
 * @brief Enum for different physics properties.
 */
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

/**
 * @class physics_component
 * @brief Component that handles physics properties and behaviors.
 */
class physics_component : public component_crtp<physics_component, owned_component>
{
public:
    /**
     * @brief Called when the component is created.
     * @param r The registry containing the component.
     * @param e The entity associated with the component.
     */
    static void on_create_component(entt::registry& r, const entt::entity e);

    /**
     * @brief Called when the component is destroyed.
     * @param r The registry containing the component.
     * @param e The entity associated with the component.
     */
    static void on_destroy_component(entt::registry& r, const entt::entity e);

    /**
     * @brief Sets whether the component uses gravity.
     * @param use_gravity True to use gravity, false otherwise.
     */
    void set_is_using_gravity(bool use_gravity);

    /**
     * @brief Checks if the component uses gravity.
     * @return True if using gravity, false otherwise.
     */
    auto is_using_gravity() const noexcept -> bool;

    /**
     * @brief Sets whether the component is kinematic.
     * @param kinematic True if kinematic, false otherwise.
     */
    void set_is_kinematic(bool kinematic);

    /**
     * @brief Checks if the component is kinematic.
     * @return True if kinematic, false otherwise.
     */
    auto is_kinematic() const noexcept -> bool;

    /**
     * @brief Sets the mass of the component.
     * @param mass The mass to set.
     */
    void set_mass(float mass);

    /**
     * @brief Gets the mass of the component.
     * @return The mass of the component.
     */
    auto get_mass() const noexcept -> float;

    /**
     * @brief Sets whether the component is a sensor.
     * @param sensor True if sensor, false otherwise.
     */
    void set_is_sensor(bool sensor);

    /**
     * @brief Checks if the component is a sensor.
     * @return True if sensor, false otherwise.
     */
    auto is_sensor() const noexcept -> bool;

    /**
     * @brief Checks if a specific property is dirty.
     * @param id The property ID.
     * @return True if the property is dirty, false otherwise.
     */
    auto is_dirty(uint8_t id) const noexcept -> bool;

    /**
     * @brief Sets the dirty flag for a specific property.
     * @param id The property ID.
     * @param dirty True to set the property as dirty, false otherwise.
     */
    void set_dirty(uint8_t id, bool dirty) noexcept;

    /**
     * @brief Checks if a specific physics property is dirty.
     * @param prop The physics property.
     * @return True if the property is dirty, false otherwise.
     */
    auto is_property_dirty(physics_property prop) const noexcept -> bool;

    /**
     * @brief Checks if any properties are dirty.
     * @return True if any properties are dirty, false otherwise.
     */
    auto are_any_properties_dirty() const noexcept -> bool;

    /**
     * @brief Checks if all properties are dirty.
     * @return True if all properties are dirty, false otherwise.
     */
    auto are_all_properties_dirty() const noexcept -> bool;

    /**
     * @brief Sets the dirty flag for a specific physics property.
     * @param prop The physics property.
     * @param dirty True to set the property as dirty, false otherwise.
     */
    void set_property_dirty(physics_property prop, bool dirty) noexcept;

    /**
     * @brief Gets the count of shapes.
     * @return The number of shapes.
     */
    auto get_shapes_count() const -> size_t;

    /**
     * @brief Gets a shape by its index.
     * @param index The index of the shape.
     * @return A constant reference to the shape.
     */
    auto get_shape_by_index(size_t index) const -> const physics_compound_shape&;

    /**
     * @brief Sets a shape by its index.
     * @param index The index of the shape.
     * @param shape The shape to set.
     */
    void set_shape_by_index(size_t index, const physics_compound_shape& shape);

    /**
     * @brief Gets all shapes.
     * @return A constant reference to the vector of shapes.
     */
    auto get_shapes() const -> const std::vector<physics_compound_shape>&;

    /**
     * @brief Sets the shapes.
     * @param shape The vector of shapes to set.
     */
    void set_shapes(const std::vector<physics_compound_shape>& shape);

    /**
     * @brief Gets the material of the component.
     * @return The asset handle to the material.
     */
    auto get_material() const -> const asset_handle<physics_material>&;

    /**
     * @brief Sets the material of the component.
     * @param material The material to set.
     */
    void set_material(const asset_handle<physics_material>& material);

    /**
     * @brief Applies an impulse to the component.
     * @param impulse The impulse vector.
     */
    void apply_impulse(const math::vec3& impulse);

    /**
     * @brief Applies a torque impulse to the component.
     * @param torque_impulse The torque impulse vector.
     */
    void apply_torque_impulse(const math::vec3& torque_impulse);

    /**
     * @brief Clears kinematic velocities.
     */
    void clear_kinematic_velocities();

private:
    /**
     * @brief Called when the gravity setting is changed.
     */
    void on_change_gravity();

    /**
     * @brief Called when the mass is changed.
     */
    void on_change_mass();

    /**
     * @brief Called when the kinematic setting is changed.
     */
    void on_change_kind();

    /**
     * @brief Called when the shape is changed.
     */
    void on_change_shape();

    /**
     * @brief Called when the material is changed.
     */
    void on_change_material();

    /**
     * @brief Called when the sensor setting is changed.
     */
    void on_change_sensor();

    bool is_kinematic_{};     ///< Indicates if the component is kinematic.
    bool is_using_gravity_{}; ///< Indicates if the component uses gravity.
    bool is_sensor_{};        ///< Indicates if the component is a sensor.
    float mass_{1};           ///< The mass of the component.

    asset_handle<physics_material> material_{};            ///< The material of the component.
    std::vector<physics_compound_shape> compound_shape_{}; ///< The vector of compound shapes.

    std::bitset<static_cast<std::underlying_type_t<physics_property>>(physics_property::count)>
        dirty_properties_; ///< Bitset for dirty properties.
    std::bitset<8> dirty_; ///< Bitset for general dirty flags.
};

} // namespace ace
