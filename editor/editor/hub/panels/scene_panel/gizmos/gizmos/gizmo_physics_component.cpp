#include "gizmo_physics_component.h"
#include <engine/ecs/components/transform_component.h>
#include <engine/physics/gizmos/gizmos.h>
namespace ace
{

void gizmo_physics_component::draw(rtti::context& ctx, rttr::variant& var, const camera& cam, gfx::dd_raii& dd)
{
    auto& data = *var.get_value<physics_component*>();

    auto owner = data.get_owner();
    const auto& transform = owner.get<transform_component>();
    const auto& shape = data.get_shapes();
    const auto& world_transform = transform.get_transform_global();

    DebugDrawEncoderScopePush scope(dd.encoder);
    uint32_t color = 0xff00ff00;

    dd.encoder.setColor(color);
    dd.encoder.setWireframe(true);
    dd.encoder.pushTransform(&world_transform);
    ::ace::draw(dd.encoder, shape);
    dd.encoder.popTransform();
}
} // namespace ace
