#include "gizmo_physics_component.h"
#include <engine/physics/backend/bullet/bullet_backend.h>
#include <engine/physics/backend/edyn/edyn_backend.h>

namespace ace
{


void gizmo_physics_component::draw(rtti::context& ctx, rttr::variant& var, const camera& cam, gfx::dd_raii& dd)
{
    auto& data = *var.get_value<physics_component*>();

    bullet_backend::draw_gizmo(data, cam, dd);
    edyn_backend::draw_gizmo(data, cam, dd);
}
} // namespace ace
