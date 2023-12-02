#pragma once

#include "inspector.h"
#include <context/context.hpp>

namespace ace
{
struct inspector_registry
{
    inspector_registry()
    {
        auto inspector_types = rttr::type::get<inspector>().get_derived_classes();
        for(auto& inspector_type : inspector_types)
        {
            auto inspected_type_var = inspector_type.get_metadata("inspected_type");
            if(inspected_type_var)
            {
                auto inspected_type = inspected_type_var.get_value<rttr::type>();
                auto inspector_var = inspector_type.create();
                if(inspector_var)
                {
                    type_map[inspected_type] = inspector_var.get_value<std::shared_ptr<inspector>>();
                }
            }
        }
    }
    std::unordered_map<rttr::type, std::shared_ptr<inspector>> type_map;
};


rttr::variant get_meta_empty(const rttr::variant& other);
bool inspect_property(rttr::instance& object, const rttr::property& prop);
bool inspect_var(rtti::context& ctx,
                 rttr::variant& var,
                 const var_info& info = {},
                 const inspector::meta_getter& get_metadata = get_meta_empty);

bool inspect_var_properties(rtti::context& ctx,
                 rttr::variant& var,
                 const var_info& info = {},
                 const inspector::meta_getter& get_metadata = get_meta_empty);

bool inspect_array(rtti::context& ctx,
                   rttr::variant& var,
                   const rttr::property& prop,
                   const var_info& info = {},
                   const inspector::meta_getter& get_metadata = get_meta_empty);
bool inspect_associative_container(rtti::context& ctx, rttr::variant& var, const rttr::property& prop, const var_info& info = {});
bool inspect_enum(rtti::context& ctx, rttr::variant& var, rttr::enumeration& data, const var_info& info = {});
} // namespace ace
