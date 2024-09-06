#pragma once

#include "inspector.h"
#include <context/context.hpp>

namespace ace
{
struct inspector_registry
{
    inspector_registry();

    std::unordered_map<rttr::type, std::shared_ptr<inspector>> type_map;
};

auto get_meta_empty(const rttr::variant& other) -> rttr::variant;
auto inspect_property(rttr::instance& object, const rttr::property& prop) -> inspect_result;
auto inspect_var(rtti::context& ctx,
                 rttr::variant& var,
                 const var_info& info = {},
                 const inspector::meta_getter& get_metadata = get_meta_empty) -> inspect_result;

auto inspect_var_properties(rtti::context& ctx,
                            rttr::variant& var,
                            const var_info& info = {},
                            const inspector::meta_getter& get_metadata = get_meta_empty) -> inspect_result;

auto inspect_array(rtti::context& ctx,
                   rttr::variant& var,
                   const rttr::property& prop,
                   const var_info& info = {},
                   const inspector::meta_getter& get_metadata = get_meta_empty) -> inspect_result;
auto inspect_associative_container(rtti::context& ctx,
                                   rttr::variant& var,
                                   const rttr::property& prop,
                                   const var_info& info = {},
                                   const inspector::meta_getter& get_metadata = get_meta_empty) -> inspect_result;
auto inspect_enum(rtti::context& ctx, rttr::variant& var, rttr::enumeration& data, const var_info& info = {}) -> inspect_result;

template<typename T>
auto inspect(rtti::context& ctx, T* obj) -> inspect_result
{
    rttr::variant var = obj;
    return inspect_var(ctx, var);
}

template<typename T>
auto inspect(rtti::context& ctx, T& obj) -> inspect_result
{
    return inspect(ctx, &obj);
}

} // namespace ace
