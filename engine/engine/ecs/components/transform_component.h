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
    void resolve_transform_global() noexcept;

    /**
     * @brief Gets the global transform.
     * @return A constant reference to the global transform.
     */
    auto get_transform_global() const noexcept -> const math::transform&;

    /**
     * @brief Sets the global transform.
     * @param trans The global transform to set.
     */
    void set_transform_global(const math::transform& trans) noexcept;

    /**
     * @brief Gets the local transform.
     * @return A constant reference to the local transform.
     */
    auto get_transform_local() const noexcept -> const math::transform&;

    /**
     * @brief Sets the local transform.
     * @param trans The local transform to set.
     */
    void set_transform_local(const math::transform& trans) noexcept;

    //---------------------------------------------
    /// TRANSLATION
    //---------------------------------------------

    /**
     * @brief Gets the global position.
     * @return A constant reference to the global position.
     */
    auto get_position_global() const noexcept -> const math::vec3&;

    /**
     * @brief Sets the global position.
     * @param position The global position to set.
     */
    void set_position_global(const math::vec3& position) noexcept;

    /**
     * @brief Moves the component by a specified amount globally.
     * @param amount The amount to move by.
     */
    void move_by_global(const math::vec3& amount) noexcept;

    /**
     * @brief Resets the global position to the origin.
     */
    void reset_position_global() noexcept;

    /**
     * @brief Gets the local position.
     * @return A constant reference to the local position.
     */
    auto get_position_local() const noexcept -> const math::vec3&;

    /**
     * @brief Sets the local position.
     * @param position The local position to set.
     */
    void set_position_local(const math::vec3& position) noexcept;

    /**
     * @brief Moves the component by a specified amount locally.
     * @param amount The amount to move by.
     */
    void move_by_local(const math::vec3& amount) noexcept;

    /**
     * @brief Resets the local position to the origin.
     */
    void reset_position_local() noexcept;

    //---------------------------------------------
    /// ROTATION
    //---------------------------------------------

    /**
     * @brief Gets the global rotation.
     * @return A constant reference to the global rotation.
     */
    auto get_rotation_global() const noexcept -> const math::quat&;

    /**
     * @brief Sets the global rotation.
     * @param rotation The global rotation to set.
     */
    void set_rotation_global(const math::quat& rotation) noexcept;

    /**
     * @brief Rotates the component by a specified amount globally.
     * @param rotation The rotation amount.
     */
    void rotate_by_global(const math::quat& rotation) noexcept;

    /**
     * @brief Resets the global rotation to the default orientation.
     */
    void reset_rotation_global() noexcept;

    /**
     * @brief Gets the local rotation.
     * @return A constant reference to the local rotation.
     */
    auto get_rotation_local() const noexcept -> const math::quat&;

    /**
     * @brief Sets the local rotation.
     * @param rotation The local rotation to set.
     */
    void set_rotation_local(const math::quat& rotation) noexcept;

    /**
     * @brief Rotates the component by a specified amount locally.
     * @param rotation The rotation amount.
     */
    void rotate_by_local(const math::quat& rotation) noexcept;

    /**
     * @brief Resets the local rotation to the default orientation.
     */
    void reset_rotation_local() noexcept;

    /**
     * @brief Gets the global rotation in Euler angles.
     * @return The global rotation in Euler angles.
     */
    auto get_rotation_euler_global() const noexcept -> math::vec3;

    /**
     * @brief Sets the global rotation in Euler angles.
     * @param rotation The global rotation in Euler angles.
     */
    void set_rotation_euler_global(math::vec3 rotation) noexcept;

    /**
     * @brief Rotates the component by a specified amount in Euler angles globally.
     * @param rotation The rotation amount in Euler angles.
     */
    void rotate_by_euler_global(math::vec3 rotation) noexcept;

    /**
     * @brief Gets the local rotation in Euler angles.
     * @return The local rotation in Euler angles.
     */
    auto get_rotation_euler_local() const noexcept -> math::vec3;

    /**
     * @brief Sets the local rotation in Euler angles.
     * @param rotation The local rotation in Euler angles.
     */
    void set_rotation_euler_local(math::vec3 rotation) noexcept;

    /**
     * @brief Rotates the component by a specified amount in Euler angles locally.
     * @param rotation The rotation amount in Euler angles.
     */
    void rotate_by_euler_local(math::vec3 rotation) noexcept;

    /**
     * @brief Rotates the component around a specified axis globally.
     * @param degrees The rotation amount in degrees.
     * @param axis The axis to rotate around.
     */
    void rotate_axis_global(float degrees, const math::vec3& axis) noexcept;

    /**
     * @brief Orients the component to look at a specified point.
     * @param point The point to look at.
     */
    void look_at(const math::vec3& point) noexcept;

    //---------------------------------------------
    /// SCALE
    //---------------------------------------------

    /**
     * @brief Gets the global scale.
     * @return A constant reference to the global scale.
     */
    auto get_scale_global() const noexcept -> const math::vec3&;

    /**
     * @brief Sets the global scale.
     * @param scale The global scale to set.
     */
    void set_scale_global(const math::vec3& scale) noexcept;

    /**
     * @brief Scales the component by a specified amount globally.
     * @param scale The scaling amount.
     */
    void scale_by_global(const math::vec3& scale) noexcept;

    /**
     * @brief Resets the global scale to the default value.
     */
    void reset_scale_global() noexcept;

    /**
     * @brief Gets the local scale.
     * @return A constant reference to the local scale.
     */
    auto get_scale_local() const noexcept -> const math::vec3&;

    /**
     * @brief Sets the local scale.
     * @param scale The local scale to set.
     */
    void set_scale_local(const math::vec3& scale) noexcept;

    /**
     * @brief Scales the component by a specified amount locally.
     * @param scale The scaling amount.
     */
    void scale_by_local(const math::vec3& scale) noexcept;

    /**
     * @brief Resets the local scale to the default value.
     */
    void reset_scale_local() noexcept;

    //---------------------------------------------
    /// SKEW
    //---------------------------------------------

    /**
     * @brief Gets the global skew.
     * @return A constant reference to the global skew.
     */
    auto get_skew_global() const noexcept -> const math::vec3&;

    /**
     * @brief Sets the global skew.
     * @param s The global skew to set.
     */
    void set_skew_global(const math::vec3& s) noexcept;

    /**
     * @brief Gets the local skew.
     * @return A constant reference to the local skew.
     */
    auto get_skew_local() const noexcept -> const math::vec3&;

    /**
     * @brief Sets the local skew.
     * @param s The local skew to set.
     */
    void set_skew_local(const math::vec3& s) noexcept;

    //---------------------------------------------
    /// PERSPECTIVE
    //---------------------------------------------

    /**
     * @brief Gets the global perspective.
     * @return A constant reference to the global perspective.
     */
    auto get_perspective_global() const noexcept -> const math::vec4&;

    /**
     * @brief Sets the global perspective.
     * @param p The global perspective to set.
     */
    void set_perspective_global(const math::vec4& p) noexcept;

    /**
     * @brief Gets the local perspective.
     * @return A constant reference to the local perspective.
     */
    auto get_perspective_local() const noexcept -> const math::vec4&;

    /**
     * @brief Sets the local perspective.
     * @param p The local perspective to set.
     */
    void set_perspective_local(const math::vec4& p) noexcept;

    //---------------------------------------------
    /// BASIS
    //---------------------------------------------

    /**
     * @brief Gets the global X-axis.
     * @return The global X-axis.
     */
    auto get_x_axis_global() const noexcept -> math::vec3;

    /**
     * @brief Gets the local X-axis.
     * @return The local X-axis.
     */
    auto get_x_axis_local() const noexcept -> math::vec3;

    /**
     * @brief Gets the global Y-axis.
     * @return The global Y-axis.
     */
    auto get_y_axis_global() const noexcept -> math::vec3;

    /**
     * @brief Gets the local Y-axis.
     * @return The local Y-axis.
     */
    auto get_y_axis_local() const noexcept -> math::vec3;

    /**
     * @brief Gets the global Z-axis.
     * @return The global Z-axis.
     */
    auto get_z_axis_global() const noexcept -> math::vec3;

    /**
     * @brief Gets the local Z-axis.
     * @return The local Z-axis.
     */
    auto get_z_axis_local() const noexcept -> math::vec3;

    //---------------------------------------------
    /// SPACE UTILS
    //---------------------------------------------

    /**
     * @brief Converts a point to local space.
     * @param point The point in global space.
     * @return The point in local space.
     */
    auto to_local(const math::vec3& point) const noexcept -> math::vec3;

    //---------------------------------------------
    /// RELATIONSHIP
    //---------------------------------------------

    /**
     * @brief Gets the parent entity.
     * @return A handle to the parent entity.
     */
    auto get_parent() const noexcept -> entt::handle;

    /**
     * @brief Sets the parent entity.
     * @param parent A handle to the parent entity.
     * @param params Parameters for setting the parent.
     * @return True if the parent was set successfully, otherwise false.
     */
    auto set_parent(const entt::handle& parent, bool global_stays = true) -> bool;

    /**
     * @brief Gets the child entities.
     * @return A constant reference to the vector of child entity handles.
     */
    auto get_children() const noexcept -> const std::vector<entt::handle>&;

    /**
     * @brief Sets the child entities.
     * @param children A vector of child entity handles.
     */
    void set_children(const std::vector<entt::handle>& children);

    /**
     * @brief Sorts the child entities.
     */
    void sort_children() noexcept;

    /**
     * @brief Sets the dirty flag.
     * @param dirty The dirty flag to set.
     */
    void set_dirty(bool dirty) noexcept;

    /**
     * @brief Checks if the component is dirty.
     * @return True if the component is dirty, otherwise false.
     */
    auto is_dirty() const noexcept -> bool;

    /**
     * @brief Sets the dirty flag for a specific index.
     * @param id The index of the flag.
     * @param dirty The dirty flag to set.
     */
    void set_dirty(uint8_t id, bool dirty) noexcept;

    /**
     * @brief Checks if the component is dirty for a specific index.
     * @param id The index of the flag.
     * @return True if the component is dirty for the specified index, otherwise false.
     */
    auto is_dirty(uint8_t id) const noexcept -> bool;

    /**
     * @brief Clears the relationships of the component.
     */
    void _clear_relationships();

    std::shared_ptr<int> sentinel = std::make_shared<int>();

private:
    /**
     * @brief Sets the owner of the component.
     * @param owner A handle to the owner entity.
     */
    void set_owner(entt::handle owner) ;

    /**
     * @brief Applies a transformation to the component.
     * @param tr The transformation to apply.
     */
    void apply_transform(const math::transform& tr) noexcept;

    /**
     * @brief Computes the inverse of the parent's transform.
     * @param parent A handle to the parent entity.
     * @return The inverse of the parent's transform.
     */
    auto inverse_parent_transform(const entt::handle& parent) noexcept -> math::transform;

    /**
     * @brief Called when the transform is set to dirty.
     * @param dirty The dirty flag.
     */
    void on_dirty_transform(bool dirty) noexcept;

    /**
     * @brief Resolves the global transform value.
     * @return The global transform.
     */
    auto resolve_global_value_transform() const noexcept -> math::transform;

    /**
     * @brief Attaches a child entity.
     * @param child A handle to the child entity.
     * @param child_transform The transform component of the child.
     */
    void attach_child(const entt::handle& child, transform_component& child_transform) ;

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
    std::vector<entt::handle> children_{};

    /**
     * @struct local_global_property
     * @brief Template structure for managing local and global properties.
     */
    template<typename T, typename Owner, void (Owner::*on_dirty)(bool), T (Owner::*resolve_global_value)() const, bool auto_resolve>
    struct local_global_property
    {
        /**
         * @brief Sets the value and marks the property as dirty.
         * @param owner The owner component.
         * @param val The value to set.
         * @return True if the value was set successfully, otherwise false.
         */
        auto set_value(Owner* owner, const T& val) noexcept -> bool
        {
            local = val;

            set_dirty(owner, true);

            return true;
        }

        /**
         * @brief Gets the local value.
         * @param owner The owner component.
         * @return A constant reference to the local value.
         */
        auto get_value(const Owner* owner) const noexcept -> const T&
        {
            return local;
        }

        /**
         * @brief Gets a reference to the local value.
         * @param owner The owner component.
         * @return A reference to the local value.
         */
        auto value(Owner* owner) noexcept -> T&
        {
            set_dirty(owner, true);

            return local;
        }

        /**
         * @brief Sets the dirty flag.
         * @param owner The owner component.
         * @param flag The dirty flag.
         */
        void set_dirty(Owner* owner, bool flag) const noexcept
        {
            if(dirty == flag)
            {
                return;
            }
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
        auto get_global_value(const Owner* owner, bool force) const noexcept -> const T&
        {
            if(force || auto_resolve && dirty)
            {
                assert(owner);
                global = (owner->*resolve_global_value)();

                set_dirty(const_cast<Owner*>(owner), false);
            }

            return global;
        }

        auto has_auto_resolve() const noexcept -> bool
        {
            return auto_resolve;
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
                                                     &transform_component::resolve_global_value_transform,
                                                     true>;

    ///< Transform property.
    property_transform transform_{};
    ///< Bitset for transform dirty flags.
    std::bitset<32> transform_dirty_{};
};

} // namespace ace
