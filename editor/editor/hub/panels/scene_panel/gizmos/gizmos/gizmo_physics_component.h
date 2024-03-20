#pragma once
#include "gizmo.h"

#include <engine/physics/ecs/components/physics_component.h>

namespace ace
{
struct gizmo_physics_component : public gizmo
{
    REFLECTABLEV(gizmo_physics_component, gizmo)

    void draw(rtti::context& ctx, rttr::variant& var, const camera& cam, gfx::dd_raii& dd);
};

GIZMO_REFLECT(gizmo_physics_component, physics_component)

} // namespace ace
