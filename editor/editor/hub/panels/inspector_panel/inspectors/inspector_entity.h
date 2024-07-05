#pragma once
#include "inspector.h"

#include <engine/ecs/ecs.h>

namespace ace
{
struct inspector_entity : public inspector
{
    REFLECTABLEV(inspector_entity, inspector)

    inspect_result inspect(rtti::context& ctx, rttr::variant& var, const var_info& info, const meta_getter& get_metadata);

private:
    ImGuiTextFilter filter_;
};

INSPECTOR_REFLECT(inspector_entity, entt::handle)
} // namespace ace
