#pragma once

#include "basic_component.h"
#include <math/math.h>

namespace ace
{

struct root_component : component_crtp<root_component>
{
};

struct set_parent_params
{
    bool global_transform_stays = false;
    bool local_transform_stays = false;
};

class transform_component : public component_crtp<transform_component, owned_component>
{
public:
    transform_component();
    ~transform_component();

    void set_owner(entt::handle owner);

    //---------------------------------------------
    /// TRANSFORMS
    //---------------------------------------------
    auto get_transform_global() const -> const math::transform&;
    void set_transform_global(const math::transform& trans);
    auto get_transform_local() const -> const math::transform&;
    void set_transform_local(const math::transform& trans);

    //---------------------------------------------
    /// TRANSLATION
    //---------------------------------------------
    auto get_position_global() const -> const math::vec3&;
    void set_position_global(const math::vec3& position);
    void move_by_global(const math::vec3& amount);
    void reset_position_global();

    auto get_position_local() const -> const math::vec3&;
    void set_position_local(const math::vec3& position);
    void move_by_local(const math::vec3& amount);
    void reset_position_local();

    //---------------------------------------------
    /// ROTATION
    //---------------------------------------------
    auto get_rotation_global() const -> const math::quat&;
    void set_rotation_global(const math::quat& rotation);
    void reset_rotation_global();

    auto get_rotation_local() const -> const math::quat&;
    void set_rotation_local(const math::quat& rotation);
    void reset_rotation_local();

    auto get_rotation_euler_global() const -> math::vec3;
    void set_rotation_euler_global(math::vec3 rotation);
    void rotate_by_euler_global(math::vec3 rotation);

    auto get_rotation_euler_local() const -> math::vec3;
    void set_rotation_euler_local(math::vec3 rotation);
    void rotate_by_euler_local(math::vec3 rotation);

    void rotate_axis_global(float degrees, const math::vec3& axis);
    void look_at(const math::vec3& point);

    //---------------------------------------------
    /// SCALE
    //---------------------------------------------
    auto get_scale_global() const -> const math::vec3&;
    void set_scale_global(const math::vec3& scale);
    void scale_by_global(const math::vec3& scale);
    void reset_scale_global();

    auto get_scale_local() const -> const math::vec3&;
    void set_scale_local(const math::vec3& scale);
    void scale_by_local(const math::vec3& scale);
    void reset_scale_local();

    //---------------------------------------------
    /// SKEW
    //---------------------------------------------
    auto get_skew_global() const -> const math::vec3&;
    void set_skew_global(const math::vec3& s);

    auto get_skew_local() const -> const math::vec3&;
    void set_skew_local(const math::vec3& s);

    //---------------------------------------------
    /// PERSPECTIVE
    //---------------------------------------------
    auto get_perspective_global() const -> const math::vec4&;
    void set_perspective_global(const math::vec4& p);

    auto get_perspective_local() const -> const math::vec4&;
    void set_perspective_local(const math::vec4& p);

    //---------------------------------------------
    /// BASIS
    //---------------------------------------------
    auto get_x_axis_global() const -> math::vec3;
    auto get_x_axis_local() const -> math::vec3;

    auto get_y_axis_global() const -> math::vec3;
    auto get_y_axis_local() const -> math::vec3;

    auto get_z_axis_global() const -> math::vec3;
    auto get_z_axis_local() const -> math::vec3;

    //---------------------------------------------
    /// SPACE UTILS
    //---------------------------------------------

    auto to_local(const math::vec3& point) const -> math::vec3;

    //---------------------------------------------
    /// RELATIONSHIP
    //---------------------------------------------

    auto get_parent() const -> entt::handle;
    auto set_parent(const entt::handle& parent, set_parent_params params = {}) -> bool;
    auto get_children() const -> const std::vector<entt::handle>&;
    void sort_children();

    void set_dirty(bool dirty) const;
    auto is_dirty() const -> bool;

private:
    void apply_transform(const math::transform& tr);
    auto inverse_parent_transform(const entt::handle& parent) -> math::transform;

    void on_dirty_transform(bool dirty);
    auto resolve_global_value_transform() -> math::transform;

    void attach_child(const entt::handle& child, transform_component& child_transform);
    auto remove_child(const entt::handle& child, transform_component& child_transform) -> bool;

    int32_t sort_index_{-1};

    entt::handle parent_{};
    std::vector<entt::handle> children_;

    template<typename T, typename Owner, void (Owner::*on_dirty)(bool), T (Owner::*resolve_global_value)()>
    struct local_global_property
    {
        // static_assert(on_dirty && resolve_global_value, "Expects a valid callbacks.");

        local_global_property() = default;
        local_global_property(const local_global_property&) = delete;
        local_global_property(local_global_property&&) = delete;
        local_global_property& operator=(const local_global_property&) = delete;
        local_global_property& operator=(local_global_property&&) = delete;

        auto set_value(const T& val, bool force_dirty = false) -> bool
        {
            if(local == val)
            {
                if(force_dirty)
                {
                    set_dirty(true);
                }
                return false;
            }
            local = val;

            set_dirty(true);

            return true;
        }

        auto get_value() const -> const T&
        {
            return local;
        }

        auto value() -> T&
        {
            set_dirty(true);

            return local;
        }

        void set_dirty(bool flag) const
        {
            dirty = flag;

            if(dirty)
            {
                assert(owner);
                (owner->*on_dirty)(flag);
            }
        }

        auto get_global_value() const -> const T&
        {
            if(dirty)
            {
                assert(owner);
                global = (owner->*resolve_global_value)();

                set_dirty(false);
            }

            return global;
        }

        mutable Owner* owner{};
        T local{};
        mutable T global{};
        mutable bool dirty{true};
    };

    /// Transform property
    using property_transform = local_global_property<math::transform,
                                                     transform_component,
                                                     &transform_component::on_dirty_transform,
                                                     &transform_component::resolve_global_value_transform>;
    property_transform transform_;
};
} // namespace ace
