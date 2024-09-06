#pragma once
#include "inspector.h"

#include <math/math.h>

namespace ace
{
struct inspector_vec2 : public inspector
{
    REFLECTABLEV(inspector_vec2, inspector)
    inspect_result inspect(rtti::context& ctx, rttr::variant& var, const var_info& info, const meta_getter& get_metadata);
};
REFLECT_INSPECTOR_INLINE(inspector_vec2, math::vec2)

struct inspector_vec3 : public inspector
{
    REFLECTABLEV(inspector_vec3, inspector)
    inspect_result inspect(rtti::context& ctx, rttr::variant& var, const var_info& info, const meta_getter& get_metadata);
};
REFLECT_INSPECTOR_INLINE(inspector_vec3, math::vec3)

struct inspector_vec4 : public inspector
{
    REFLECTABLEV(inspector_vec4, inspector)
    inspect_result inspect(rtti::context& ctx, rttr::variant& var, const var_info& info, const meta_getter& get_metadata);
};
REFLECT_INSPECTOR_INLINE(inspector_vec4, math::vec4)

struct inspector_color : public inspector
{
    REFLECTABLEV(inspector_color, inspector)
    inspect_result inspect(rtti::context& ctx, rttr::variant& var, const var_info& info, const meta_getter& get_metadata);
};
REFLECT_INSPECTOR_INLINE(inspector_color, math::color)

struct inspector_quaternion : public inspector
{
    REFLECTABLEV(inspector_quaternion, inspector)
    inspect_result inspect(rtti::context& ctx, rttr::variant& var, const var_info& info, const meta_getter& get_metadata);
};
REFLECT_INSPECTOR_INLINE(inspector_quaternion, math::quat)

struct inspector_transform : public inspector
{
    REFLECTABLEV(inspector_transform, inspector)

    void before_inspect(const rttr::property& prop);
    inspect_result inspect(rtti::context& ctx, rttr::variant& var, const var_info& info, const meta_getter& get_metadata);
};
REFLECT_INSPECTOR_INLINE(inspector_transform, math::transform)
} // namespace ace
