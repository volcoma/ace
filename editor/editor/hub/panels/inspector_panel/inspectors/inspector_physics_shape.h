#pragma once
#include "inspector.h"

#include <engine/ecs/components/physics_component.h>

namespace ace
{

struct inspector_physics_compound_shape : public inspector
{
    REFLECTABLEV(inspector_physics_compound_shape, inspector)

    bool inspect(rtti::context& ctx, rttr::variant& var, const var_info& info, const meta_getter& get_metadata);
};

INSPECTOR_REFLECT(inspector_physics_compound_shape, physics_compound_shape)
} // namespace ace
