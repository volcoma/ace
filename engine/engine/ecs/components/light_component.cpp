#include "light_component.h"

namespace ace
{
const light& light_component::get_light() const
{
    return light_;
}

void light_component::set_light(const light& l)
{
    light_ = l;
}

auto light_component::get_bounds_sphere_impl(const math::vec3* light_direction) const -> math::bsphere
{
    math::bsphere result;

    if(light_.type == light_type::point)
    {
        result = math::bsphere(math::vec3(0.0f, 0.0f, 0.0f), light_.point_data.range);
    }
    else if(light_.type == light_type::spot)
    {
        float range = light_.spot_data.get_range();

        if(light_direction)
        {
            float clamped_inner_cone_angle =
                math::radians(math::clamp(light_.spot_data.get_inner_angle(), 0.0f, 89.0f));
            float clamped_outer_cone_angle = math::clamp(math::radians(light_.spot_data.get_outer_angle()),
                                                         clamped_inner_cone_angle + 0.001f,
                                                         math::radians(89.0f) + 0.001f);
            float cos_outer_cone = math::cos(clamped_outer_cone_angle);
            // Use the law of cosines to find the distance to the furthest edge of the
            // spotlight cone from a
            // position that is halfway down the spotlight direction
            const float radius = math::sqrt(1.25f * range * range - range * range * cos_outer_cone);
            math::vec3 center = math::vec3(0.0f, 0.0f, 0.0f) + 0.5f * (*light_direction) * range;

            result = math::bsphere(center, radius);
        }
        else
        {
            result = math::bsphere(math::vec3(0.0f, 0.0f, 0.0f), range);
        }
    }
    else
    {
        result = math::bsphere(math::vec3(0.0f, 0.0f, 0.0f), 999999999.0f);
    }

    return result;
}

auto light_component::get_bounds_sphere() const -> math::bsphere
{
    return get_bounds_sphere_impl(nullptr);
}

auto light_component::get_bounds_sphere_precise(const math::vec3& light_direction) const -> math::bsphere
{
    return get_bounds_sphere_impl(&light_direction);
}

auto light_component::get_bounds() const -> math::bbox
{
    auto sphere = get_bounds_sphere();
    math::bbox result;
    result.from_sphere(sphere.position, sphere.radius);
    return result;
}

auto light_component::get_bounds_precise(const math::vec3& light_direction) const -> math::bbox
{
    auto sphere = get_bounds_sphere_precise(light_direction);
    math::bbox result;
    result.from_sphere(sphere.position, sphere.radius);
    return result;
}

int light_component::compute_projected_sphere_rect(irect32_t& rect,
                                                   const math::vec3& light_position,
                                                   const math::vec3& light_direction,
                                                   const math::vec3& view_origin,
                                                   const math::transform& view,
                                                   const math::transform& proj)
{
    if(light_.type == light_type::point)
    {
        return math::compute_projected_sphere_rect(rect.left,
                                                   rect.right,
                                                   rect.top,
                                                   rect.bottom,
                                                   light_position,
                                                   light_.point_data.range,
                                                   view_origin,
                                                   view,
                                                   proj);
    }
    else if(light_.type == light_type::spot)
    {
        float range = light_.spot_data.get_range();
        float clamped_inner_cone_angle = math::radians(math::clamp(light_.spot_data.get_inner_angle(), 0.0f, 89.0f));
        float clamped_outer_cone_angle = math::clamp(math::radians(light_.spot_data.get_outer_angle()),
                                                     clamped_inner_cone_angle + 0.001f,
                                                     math::radians(89.0f) + 0.001f);
        float cos_outer_cone = math::cos(clamped_outer_cone_angle);
        // Use the law of cosines to find the distance to the furthest edge of the
        // spotlight cone from a
        // position that is halfway down the spotlight direction
        const float radius = math::sqrt(1.25f * range * range - range * range * cos_outer_cone);
        math::vec3 center = light_position + 0.5f * light_direction * range;

        return math::compute_projected_sphere_rect(rect.left,
                                                   rect.right,
                                                   rect.top,
                                                   rect.bottom,
                                                   center,
                                                   radius,
                                                   view_origin,
                                                   view,
                                                   proj);
    }
    else
    {
        return 1;
    }
}

auto light_component::get_shadowmap_generator() -> shadow::shadowmap_generator&
{
    return shadowmap_generator_;
}

auto skylight_component::get_mode() const noexcept -> const sky_mode&
{
    return mode_;
}
void skylight_component::set_mode(const sky_mode& mode)
{
    mode_ = mode;
}

} // namespace ace
