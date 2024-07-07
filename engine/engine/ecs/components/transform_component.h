#pragma once

#include "basic_component.h"
#include <bitset>
#include <math/math.h>

namespace ace
{

/**
 * @struct root_component
 * @brief Root component structure for the ACE framework, serves as the base component.
 */
struct root_component : public component_crtp<root_component>
{
};

/**
 * @struct set_parent_params
 * @brief Parameters for setting a parent in a transform hierarchy.
 */
struct set_parent_params
{
    /// Indicates if the global transform remains the same.
    bool global_transform_stays = true;
    /// Indicates if the local transform remains the same.
    bool local_transform_stays = false;
};

/**
 * @class transform_component
 * @brief Component that handles transformations (position, rotation, scale, etc.) in the ACE framework.
 */
class transform_component : public component_crtp<transform_component, owned_component>
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

    //---------------------------------------------
    /// TRANSFORMS
    //---------------------------------------------

    /**
     * @brief Gets the global transform.
     * @return A constant reference to the global transform.
     */
    auto get_transform_global() const -> const math::transform&;

    /**
     * @brief Sets the global transform.
     * @param trans The global transform to set.
     */
    void set_transform_global(const math::transform& trans);

    /**
     * @brief Gets the local transform.
     * @return A constant reference to the local transform.
     */
    auto get_transform_local() const -> const math::transform&;

    /**
     * @brief Sets the local transform.
     * @param trans The local transform to set.
     */
    void set_transform_local(const math::transform& trans);

    //---------------------------------------------
    /// TRANSLATION
    //---------------------------------------------

    /**
     * @brief Gets the global position.
     * @return A constant reference to the global position.
     */
    auto get_position_global() const -> const math::vec3&;

    /**
     * @brief Sets the global position.
     * @param position The global position to set.
     */
    void set_position_global(const math::vec3& position);

    /**
     * @brief Moves the component by a specified amount globally.
     * @param amount The amount to move by.
     */
    void move_by_global(const math::vec3& amount);

    /**
     * @brief Resets the global position to the origin.
     */
    void reset_position_global();

    /**
     * @brief Gets the local position.
     * @return A constant reference to the local position.
     */
    auto get_position_local() const -> const math::vec3&;

    /**
     * @brief Sets the local position.
     * @param position The local position to set.
     */
    void set_position_local(const math::vec3& position);

    /**
     * @brief Moves the component by a specified amount locally.
     * @param amount The amount to move by.
     */
    void move_by_local(const math::vec3& amount);

    /**
     * @brief Resets the local position to the origin.
     */
    void reset_position_local();

    //---------------------------------------------
    /// ROTATION
    //---------------------------------------------

    /**
     * @brief Gets the global rotation.
     * @return A constant reference to the global rotation.
     */
    auto get_rotation_global() const -> const math::quat&;

    /**
     * @brief Sets the global rotation.
     * @param rotation The global rotation to set.
     */
    void set_rotation_global(const math::quat& rotation);

    /**
     * @brief Rotates the component by a specified amount globally.
     * @param rotation The rotation amount.
     */
    void rotate_by_global(const math::quat& rotation);

    /**
     * @brief Resets the global rotation to the default orientation.
     */
    void reset_rotation_global();

    /**
     * @brief Gets the local rotation.
     * @return A constant reference to the local rotation.
     */
    auto get_rotation_local() const -> const math::quat&;

    /**
     * @brief Sets the local rotation.
     * @param rotation The local rotation to set.
     */
    void set_rotation_local(const math::quat& rotation);

    /**
     * @brief Rotates the component by a specified amount locally.
     * @param rotation The rotation amount.
     */
    void rotate_by_local(const math::quat& rotation);

    /**
     * @brief Resets the local rotation to the default orientation.
     */
    void reset_rotation_local();

    /**
     * @brief Gets the global rotation in Euler angles.
     * @return The global rotation in Euler angles.
     */
    auto get_rotation_euler_global() const -> math::vec3;

    /**
     * @brief Sets the global rotation in Euler angles.
     * @param rotation The global rotation in Euler angles.
     */
    void set_rotation_euler_global(math::vec3 rotation);

    /**
     * @brief Rotates the component by a specified amount in Euler angles globally.
     * @param rotation The rotation amount in Euler angles.
     */
    void rotate_by_euler_global(math::vec3 rotation);

    /**
     * @brief Gets the local rotation in Euler angles.
     * @return The local rotation in Euler angles.
     */
    auto get_rotation_euler_local() const -> math::vec3;

    /**
     * @brief Sets the local rotation in Euler angles.
     * @param rotation The local rotation in Euler angles.
     */
    void set_rotation_euler_local(math::vec3 rotation);

    /**
     * @brief Rotates the component by a specified amount in Euler angles locally.
     * @param rotation The rotation amount in Euler angles.
     */
    void rotate_by_euler_local(math::vec3 rotation);

    /**
     * @brief Rotates the component around a specified axis globally.
     * @param degrees The rotation amount in degrees.
     * @param axis The axis to rotate around.
     */
    void rotate_axis_global(float degrees, const math::vec3& axis);

    /**
     * @brief Orients the component to look at a specified point.
     * @param point The point to look at.
     */
    void look_at(const math::vec3& point);

    //---------------------------------------------
    /// SCALE
    //---------------------------------------------

    /**
     * @brief Gets the global scale.
     * @return A constant reference to the global scale.
     */
    auto get_scale_global() const -> const math::vec3&;

    /**
     * @brief Sets the global scale.
     * @param scale The global scale to set.
     */
    void set_scale_global(const math::vec3& scale);

    /**
     * @brief Scales the component by a specified amount globally.
     * @param scale The scaling amount.
     */
    void scale_by_global(const math::vec3& scale);

    /**
     * @brief Resets the global scale to the default value.
     */
    void reset_scale_global();

    /**
     * @brief Gets the local scale.
     * @return A constant reference to the local scale.
     */
    auto get_scale_local() const -> const math::vec3&;

    /**
     * @brief Sets the local scale.
     * @param scale The local scale to set.
     */
    void set_scale_local(const math::vec3& scale);

    /**
     * @brief Scales the component by a specified amount locally.
     * @param scale The scaling amount.
     */
    void scale_by_local(const math::vec3& scale);

    /**
     * @brief Resets the local scale to the default value.
     */
    void reset_scale_local();

    //---------------------------------------------
    /// SKEW
    //---------------------------------------------

    /**
     * @brief Gets the global skew.
     * @return A constant reference to the global skew.
     */
    auto get_skew_global() const -> const math::vec3&;

    /**
     * @brief Sets the global skew.
     * @param s The global skew to set.
     */
    void set_skew_global(const math::vec3& s);

    /**
     * @brief Gets the local skew.
     * @return A constant reference to the local skew.
     */
    auto get_skew_local() const -> const math::vec3&;

    /**
     * @brief Sets the local skew.
     * @param s The local skew to set.
     */
    void set_skew_local(const math::vec3& s);

    //---------------------------------------------
    /// PERSPECTIVE
    //---------------------------------------------

    /**
     * @brief Gets the global perspective.
     * @return A constant reference to the global perspective.
     */
    auto get_perspective_global() const -> const math::vec4&;

    /**
     * @brief Sets the global perspective.
     * @param p The global perspective to set.
     */
    void set_perspective_global(const math::vec4& p);

    /**
     * @brief Gets the local perspective.
     * @return A constant reference to the local perspective.
     */
    auto get_perspective_local() const -> const math::vec4&;

    /**
     * @brief Sets the local perspective.
     * @param p The local perspective to set.
     */
    void set_perspective_local(const math::vec4& p);

    //---------------------------------------------
    /// BASIS
    //---------------------------------------------

    /**
     * @brief Gets the global X-axis.
     * @return The global X-axis.
     */
    auto get_x_axis_global() const -> math::vec3;

    /**
     * @brief Gets the local X-axis.
     * @return The local X-axis.
     */
    auto get_x_axis_local() const -> math::vec3;

    /**
     * @brief Gets the global Y-axis.
     * @return The global Y-axis.
     */
    auto get_y_axis_global() const -> math::vec3;

    /**
     * @brief Gets the local Y-axis.
     * @return The local Y-axis.
     */
    auto get_y_axis_local() const -> math::vec3;

    /**
     * @brief Gets the global Z-axis.
     * @return The global Z-axis.
     */
    auto get_z_axis_global() const -> math::vec3;

    /**
     * @brief Gets the local Z-axis.
     * @return The local Z-axis.
     */
    auto get_z_axis_local() const -> math::vec3;

    //---------------------------------------------
    /// SPACE UTILS
    //---------------------------------------------

    /**
     * @brief Converts a point to local space.
     * @param point The point in global space.
     * @return The point in local space.
     */
    auto to_local(const math::vec3& point) const -> math::vec3;

    //---------------------------------------------
    /// RELATIONSHIP
    //---------------------------------------------

    /**
     * @brief Gets the parent entity.
     * @return A handle to the parent entity.
     */
    auto get_parent() const -> entt::handle;

    /**
     * @brief Sets the parent entity.
     * @param parent A handle to the parent entity.
     * @param params Parameters for setting the parent.
     * @return True if the parent was set successfully, otherwise false.
     */
    auto set_parent(const entt::handle& parent, set_parent_params params = {}) -> bool;

    /**
     * @brief Gets the child entities.
     * @return A constant reference to the vector of child entity handles.
     */
    auto get_children() const -> const std::vector<entt::handle>&;

    /**
     * @brief Sets the child entities.
     * @param children A vector of child entity handles.
     */
    void set_children(const std::vector<entt::handle>& children);

    /**
     * @brief Sorts the child entities.
     */
    void sort_children();

    /**
     * @brief Sets the dirty flag.
     * @param dirty The dirty flag to set.
     */
    void set_dirty(bool dirty);

    /**
     * @brief Checks if the component is dirty.
     * @return True if the component is dirty, otherwise false.
     */
    auto is_dirty() const -> bool;

    /**
     * @brief Sets the dirty flag for a specific index.
     * @param id The index of the flag.
     * @param dirty The dirty flag to set.
     */
    void set_dirty(uint8_t id, bool dirty);

    /**
     * @brief Checks if the component is dirty for a specific index.
     * @param id The index of the flag.
     * @return True if the component is dirty for the specified index, otherwise false.
     */
    auto is_dirty(uint8_t id) const -> bool;

    /**
     * @brief Clears the relationships of the component.
     */
    void _clear_relationships();

private:
    /**
     * @brief Sets the owner of the component.
     * @param owner A handle to the owner entity.
     */
    void set_owner(entt::handle owner);

    /**
     * @brief Applies a transformation to the component.
     * @param tr The transformation to apply.
     */
    void apply_transform(const math::transform& tr);

    /**
     * @brief Computes the inverse of the parent's transform.
     * @param parent A handle to the parent entity.
     * @return The inverse of the parent's transform.
     */
    auto inverse_parent_transform(const entt::handle& parent) -> math::transform;

    /**
     * @brief Called when the transform is set to dirty.
     * @param dirty The dirty flag.
     */
    void on_dirty_transform(bool dirty);

    /**
     * @brief Resolves the global transform value.
     * @return The global transform.
     */
    auto resolve_global_value_transform() const -> math::transform;

    /**
     * @brief Attaches a child entity.
     * @param child A handle to the child entity.
     * @param child_transform The transform component of the child.
     */
    void attach_child(const entt::handle& child, transform_component& child_transform);

    /**
     * @brief Removes a child entity.
     * @param child A handle to the child entity.
     * @param child_transform The transform component of the child.
     * @return True if the child was removed successfully, otherwise false.
     */
    auto remove_child(const entt::handle& child, transform_component& child_transform) -> bool;

    /// The sort index for sorting children.
    int32_t sort_index_{-1};

    /// The parent entity handle.
    entt::handle parent_{};
    /// The vector of child entity handles.
    std::vector<entt::handle> children_;

    /**
     * @struct local_global_property
     * @brief Template structure for managing local and global properties.
     */
    template<typename T, typename Owner, void (Owner::*on_dirty)(bool), T (Owner::*resolve_global_value)() const>
    struct local_global_property
    {
        /**
         * @brief Sets the value and marks the property as dirty.
         * @param owner The owner component.
         * @param val The value to set.
         * @return True if the value was set successfully, otherwise false.
         */
        auto set_value(Owner* owner, const T& val) -> bool
        {
            if(local == val)
            {
                return false;
            }
            local = val;

            set_dirty(owner, true);

            return true;
        }

        /**
         * @brief Gets the local value.
         * @param owner The owner component.
         * @return A constant reference to the local value.
         */
        auto get_value(const Owner* owner) const -> const T&
        {
            return local;
        }

        /**
         * @brief Gets a reference to the local value.
         * @param owner The owner component.
         * @return A reference to the local value.
         */
        auto value(Owner* owner) -> T&
        {
            set_dirty(owner, true);

            return local;
        }

        /**
         * @brief Sets the dirty flag.
         * @param owner The owner component.
         * @param flag The dirty flag.
         */
        void set_dirty(Owner* owner, bool flag) const
        {
            dirty = flag;

            if(dirty)
            {
                assert(owner);
                (owner->*on_dirty)(flag);
            }
        }

        /**
         * @brief Gets the global value.
         * @param owner The owner component.
         * @return A constant reference to the global value.
         */
        auto get_global_value(const Owner* owner) const -> const T&
        {
            if(dirty)
            {
                assert(owner);
                global = (owner->*resolve_global_value)();

                set_dirty(const_cast<Owner*>(owner), false);
            }

            return global;
        }

        /// The local value.
        T local{};
        /// The global value.
        mutable T global{};
        /// The dirty flag.
        mutable bool dirty{true};
    };

    /**
     * @typedef property_transform
     * @brief Typedef for transform property.
     */
    using property_transform = local_global_property<math::transform,
                                                     transform_component,
                                                     &transform_component::on_dirty_transform,
                                                     &transform_component::resolve_global_value_transform>;

    ///< Transform property.
    property_transform transform_{};
    ///< Bitset for transform dirty flags.
    std::bitset<32> transform_dirty_;
};

} // namespace ace
