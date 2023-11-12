#pragma once

#include "inspector.h"
#include <context/context.hpp>

namespace ace
{
rttr::variant get_meta_empty(const rttr::variant& other);
bool inspect_property(rttr::instance& object, const rttr::property& prop);
bool inspect_var(rtti::context& ctx,
                 rttr::variant& var,
                 const var_info& info = {},
                 const inspector::meta_getter& get_metadata = get_meta_empty);
bool inspect_array(rtti::context& ctx,
                   rttr::variant& var,
                   const var_info& info = {},
                   const inspector::meta_getter& get_metadata = get_meta_empty);
bool inspect_associative_container(rtti::context& ctx, rttr::variant& var, const var_info& info = {});
bool inspect_enum(rtti::context& ctx, rttr::variant& var, rttr::enumeration& data, const var_info& info = {});
} // namespace ace
