#include "transform_component.h"

#include <logging/logging.h>

#include <algorithm>

namespace ace
{
auto check_parent(entt::handle e, entt::handle parent) -> bool
{
    if(!parent)
    {
        return true;
    }

    if(e == parent)
    {
        return false;
    }

    const auto& children = e.get<transform_component>().get_children();

    for(const auto& child : children)
    {
        if(!check_parent(child, parent))
        {
            return false;
        }
    }

    return true;
}

transform_component::transform_component()
{
    transform_.owner = this;
}

transform_component::~transform_component()
{
    if(parent_)
    {
        auto parent_transform = parent_.try_get<transform_component>();
        if(parent_transform)
        {
            parent_transform->remove_child(get_owner(), *this);
        }
    }

    for(auto& child : children_)
    {
        if(child)
        {
            child.destroy();
        }
    }
}

void transform_component::set_owner(entt::handle owner)
{
    base::set_owner(owner);
    get_owner().emplace_or_replace<root_component>();
}

//---------------------------------------------
/// TRANSFORMS
//---------------------------------------------
auto transform_component::get_transform_local() const -> const math::transform&
{
    return transform_.get_value();
}

void transform_component::set_transform_local(const math::transform& trans)
{
    transform_.set_value(trans);
}

auto transform_component::get_transform_global() const -> const math::transform&
{
    return transform_.get_global_value();
}

void transform_component::set_transform_global(const math::transform& tr)
{
    if(get_transform_global().compare(tr, math::epsilon<float>()) == 0)
    {
        return;
    }

    apply_transform(tr);
}

//---------------------------------------------
/// TRANSLATION
//---------------------------------------------
auto transform_component::get_position_global() const -> const math::vec3&
{
    return get_transform_global().get_position();
}

void transform_component::set_position_global(const math::vec3& position)
{
    const auto& this_pos = get_position_global();
    if(math::all(math::epsilonEqual(this_pos, position, math::epsilon<float>())))
    {
        return;
    }
    auto m = get_transform_global();
    m.set_position(position);

    apply_transform(m);
}

void transform_component::move_by_global(const math::vec3& amount)
{
    math::vec3 new_pos = get_position_global() + amount;
    set_position_global(new_pos);
}

void transform_component::reset_position_global()
{
    set_position_global(math::vec3(0.0f, 0.0f, 0.0f));
}

auto transform_component::get_position_local() const -> const math::vec3&
{
    return get_transform_local().get_position();
}

void transform_component::set_position_local(const math::vec3& position)
{
    const auto& this_pos = get_position_local();
    if(math::all(math::epsilonEqual(this_pos, position, math::epsilon<float>())))
    {
        return;
    }

    transform_.value().set_position(position);
}

void transform_component::move_by_local(const math::vec3& amount)
{
    math::vec3 new_pos = get_position_local();

    new_pos += get_x_axis_local() * amount.x;
    new_pos += get_y_axis_local() * amount.y;
    new_pos += get_z_axis_local() * amount.z;

    set_position_local(new_pos);
}

void transform_component::reset_position_local()
{
    set_position_local(math::vec3(0.0f, 0.0f, 0.0f));
}

//---------------------------------------------
/// ROTATION
//---------------------------------------------
auto transform_component::get_rotation_global() const -> const math::quat&
{
    return get_transform_global().get_rotation();
}

void transform_component::set_rotation_global(const math::quat& rotation)
{
    const auto& this_rotation = get_rotation_global();
    if(math::all(math::epsilonEqual(this_rotation, rotation, math::epsilon<float>())))
    {
        return;
    }

    auto m = get_transform_global();
    m.set_rotation(rotation);

    apply_transform(m);
}

void transform_component::reset_rotation_global()
{
    set_rotation_global(math::transform::quat_t{1, 0, 0, 0});
}

auto transform_component::get_rotation_local() const -> const math::quat&
{
    return get_transform_local().get_rotation();
}

void transform_component::set_rotation_local(const math::quat& rotation)
{
    const auto& this_rotation = get_rotation_local();
    if(math::all(math::epsilonEqual(this_rotation, rotation, math::epsilon<float>())))
    {
        return;
    }

    transform_.value().set_rotation(rotation);
}

void transform_component::reset_rotation_local()
{
    set_rotation_local(math::transform::quat_t{1, 0, 0, 0});
}

auto transform_component::get_rotation_euler_global() const -> math::vec3
{
    return math::degrees(math::eulerAngles(get_rotation_global()));
}

void transform_component::set_rotation_euler_global(math::vec3 rotation)
{
    set_rotation_global(math::transform::quat_t(math::radians(rotation)));
}

void transform_component::rotate_by_euler_global(math::vec3 rotation)
{
    auto m = get_transform_global();
    m.rotate(math::radians(rotation));

    set_transform_global(m);
}

auto transform_component::get_rotation_euler_local() const -> math::vec3
{
    return math::degrees(math::eulerAngles(get_rotation_local()));
}

void transform_component::set_rotation_euler_local(math::vec3 rotation)
{
    set_rotation_local(math::transform::quat_t(math::radians(rotation)));
}

void transform_component::rotate_by_euler_local(math::vec3 rotation)
{
    auto m = get_transform_local();
    m.rotate_local(math::radians(rotation));

    set_transform_local(m);
}

void transform_component::rotate_axis_global(float degrees, const math::vec3& axis)
{
    auto m = get_transform_global();
    m.rotate_axis(math::radians(degrees), axis);

    set_transform_global(m);
}

void transform_component::look_at(const math::vec3& point)
{
    auto eye = get_position_global();
    math::transform m = math::lookAt(eye, point, math::vec3{0.0f, 1.0f, 0.0f});
    m = math::inverse(m);

    set_rotation_global(m.get_rotation());
}

//---------------------------------------------
/// SCALE
//---------------------------------------------

auto transform_component::get_scale_local() const -> const math::vec3&
{
    return get_transform_local().get_scale();
}

void transform_component::scale_by_local(const math::vec3& scale)
{
    transform_.value().scale(scale);
}

auto transform_component::get_skew_local() const -> const math::vec3&
{
    return get_transform_local().get_skew();
}

auto transform_component::get_perspective_local() const -> const math::vec4&
{
    return get_transform_local().get_perspective();
}

auto transform_component::get_x_axis_local() const -> math::vec3
{
    return get_transform_local().x_unit_axis();
}

auto transform_component::get_y_axis_local() const -> math::vec3
{
    return get_transform_local().y_unit_axis();
}

auto transform_component::get_z_axis_local() const -> math::vec3
{
    return get_transform_local().z_unit_axis();
}

auto transform_component::get_x_axis_global() const -> math::vec3
{
    return get_transform_global().x_unit_axis();
}

auto transform_component::get_y_axis_global() const -> math::vec3
{
    return get_transform_global().y_unit_axis();
}

auto transform_component::get_z_axis_global() const -> math::vec3
{
    return get_transform_global().z_unit_axis();
}

auto transform_component::get_scale_global() const -> const math::vec3&
{
    return get_transform_global().get_scale();
}

void transform_component::scale_by_global(const math::vec3& scale)
{
    auto m = get_transform_global();
    m.scale(scale);

    apply_transform(m);
}

auto transform_component::get_skew_global() const -> const math::vec3&
{
    return get_transform_global().get_skew();
}

auto transform_component::get_perspective_global() const -> const math::vec4&
{
    return get_transform_global().get_perspective();
}

void transform_component::set_scale_global(const math::vec3& scale)
{
    const auto& this_scale = get_scale_global();
    if(math::all(math::epsilonEqual(this_scale, scale, math::epsilon<float>())))
    {
        return;
    }

    auto m = get_transform_global();
    m.set_scale(scale);

    apply_transform(m);
}

void transform_component::set_scale_local(const math::vec3& scale)
{
    const auto& this_scale = get_scale_local();
    if(math::all(math::epsilonEqual(this_scale, scale, math::epsilon<float>())))
    {
        return;
    }

    transform_.value().set_scale(scale);
}

void transform_component::set_skew_global(const math::vec3& skew)
{
    const auto& this_skew = get_skew_global();
    if(math::all(math::epsilonEqual(this_skew, skew, math::epsilon<float>())))
    {
        return;
    }

    auto m = get_transform_global();
    m.set_skew(skew);

    apply_transform(m);
}

void transform_component::set_skew_local(const math::vec3& skew)
{
    const auto& this_skew = get_skew_local();
    if(math::all(math::epsilonEqual(this_skew, skew, math::epsilon<float>())))
    {
        return;
    }

    transform_.value().set_skew(skew);
}

void transform_component::set_perspective_global(const math::vec4& perspective)
{
    const auto& this_perspective = get_perspective_global();
    if(math::all(math::epsilonEqual(this_perspective, perspective, math::epsilon<float>())))
    {
        return;
    }

    auto m = get_transform_global();
    m.set_perspective(perspective);

    apply_transform(m);
}

void transform_component::set_perspective_local(const math::vec4& perspective)
{
    const auto& this_perspective = get_perspective_local();
    if(math::all(math::epsilonEqual(this_perspective, perspective, math::epsilon<float>())))
    {
        return;
    }

    transform_.value().set_perspective(perspective);
}

void transform_component::reset_scale_global()
{
    set_scale_global(math::vec3{1.0f, 1.0f, 1.0f});
}

void transform_component::reset_scale_local()
{
    set_scale_local(math::vec3{1.0f, 1.0f, 1.0f});
}

auto transform_component::set_parent(const entt::handle& p, set_parent_params params) -> bool
{
    auto new_parent = p;
    auto old_parent = parent_;

    // Skip if this is a no-op.
    if(old_parent == new_parent)
    {
        return false;
    }

    // Skip if parent is our child.
    if(!check_parent(get_owner(), new_parent))
    {
        return false;
    }

    // Before we do anything, make sure that all pending math::transform
    // operations are resolved (including those applied to our parent).
    math::transform cached_transform_global;
    if(params.global_transform_stays)
    {
        cached_transform_global = get_transform_global();
    }

    parent_ = new_parent;
    set_dirty(true);

    if(params.global_transform_stays)
    {
        set_transform_global(cached_transform_global);
    }
    else
    {
        if(!params.local_transform_stays)
        {
            set_transform_local(math::transform::identity());
        }
    }

    set_dirty(true);

    if(new_parent)
    {
        new_parent.get<transform_component>().attach_child(get_owner(), *this);

        if(!old_parent)
        {
            get_owner().remove<root_component>();
        }
    }
    else
    {
        get_owner().emplace_or_replace<root_component>();
    }

    if(old_parent)
    {
        old_parent.get<transform_component>().remove_child(get_owner(), *this);
    }

    return true;
}

auto transform_component::get_parent() const -> entt::handle
{
    return parent_;
}

void transform_component::attach_child(const entt::handle& child, transform_component& child_transform)
{
    child_transform.sort_index_ = int32_t(children_.size());
    children_.push_back(child);
    sort_children();
    set_dirty(is_dirty());
}

auto transform_component::remove_child(const entt::handle& child, transform_component& child_transform) -> bool
{
    auto iter = std::remove_if(std::begin(children_),
                               std::end(children_),
                               [&child](const auto& other)
                               {
                                   return child == other;
                               });
    if(iter == std::end(children_))
    {
        return false;
    }

    assert(std::distance(iter, std::end(children_)) == 1);

    auto removed_idx = child_transform.sort_index_;

    children_.erase(iter, std::end(children_));

    // shift all bigger sort indices
    for(auto& c : children_)
    {
        auto& sort_idx = c.get<transform_component>().sort_index_;
        if(sort_idx > removed_idx)
        {
            sort_idx--;
        }
    }
    child_transform.sort_index_ = {-1};

    return true;
}

void transform_component::sort_children()
{
    std::sort(children_.begin(),
              children_.end(),
              [](const auto& lhs, const auto& rhs)
              {
                  return lhs.template get<transform_component>().sort_index_ <
                         rhs.template get<transform_component>().sort_index_;
              });
}

void transform_component::apply_transform(const math::transform& tr)
{
    auto parent = parent_;
    if(parent)
    {
        auto inv_parent_transform = inverse_parent_transform(parent);
        set_transform_local(inv_parent_transform * tr);
    }
    else
    {
        set_transform_local(tr);
    }
}

auto transform_component::inverse_parent_transform(const entt::handle& parent) -> math::transform
{
    const auto& parent_transform = parent.get<transform_component>().get_transform_global();
    return math::inverse(parent_transform);
}

auto transform_component::to_local(const math::vec3& point) const -> math::vec3
{
    return get_transform_global().inverse_transform_coord(point);
}

auto transform_component::is_dirty() const -> bool
{
    return transform_.dirty;
}

void transform_component::set_dirty(bool dirty) const
{
    transform_.set_dirty(dirty);
}

auto transform_component::get_children() const -> const std::vector<entt::handle>&
{
    return children_;
}

void transform_component::on_dirty_transform(bool dirty)
{
    for(const auto& child : get_children())
    {
        auto component = child.try_get<transform_component>();
        if(component)
        {
            component->transform_.set_dirty(dirty);
        }
    }
}

auto transform_component::resolve_global_value_transform() -> math::transform
{
    auto parent = get_parent();

    if(parent)
    {
        const auto& parent_transform = parent.get<transform_component>().get_transform_global();
        const auto& local_transform = get_transform_local();

        return parent_transform * local_transform;
    }

    return get_transform_local();
}
} // namespace ace
