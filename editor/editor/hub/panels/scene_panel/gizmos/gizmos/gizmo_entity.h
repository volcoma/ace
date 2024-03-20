#pragma once
#include "gizmo.h"

#include <engine/ecs/ecs.h>

namespace ace
{
struct gizmo_entity : public gizmo
{
    REFLECTABLEV(gizmo_entity, gizmo)

    void draw(rtti::context& ctx, rttr::variant& var, const camera& cam, gfx::dd_raii& dd);
};

GIZMO_REFLECT(gizmo_entity, entt::handle)
} // namespace ace
