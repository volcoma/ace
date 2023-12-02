#pragma once
#include "inspector.h"

#include <engine/ecs/components/light_component.h>
#include <engine/ecs/components/reflection_probe_component.h>

namespace ace
{
struct inspector_light_component : public inspector
{
    REFLECTABLEV(inspector_light_component, inspector)

    bool inspect(rtti::context& ctx, rttr::variant& var, const var_info& info, const meta_getter& get_metadata);
};

INSPECTOR_REFLECT(inspector_light_component, light_component)

struct inspector_reflection_probe_component : public inspector
{
    REFLECTABLEV(inspector_reflection_probe_component, inspector)

    bool inspect(rtti::context& ctx, rttr::variant& var, const var_info& info, const meta_getter& get_metadata);
};

INSPECTOR_REFLECT(inspector_reflection_probe_component, reflection_probe_component)
} // namespace ace
