#pragma once

#include <engine/physics/ecs/components/physics_component.h>
#include <graphics/debugdraw.h>

namespace ace
{

void draw(DebugDrawEncoder& dde, const physics_sphere_shape& sh);

void draw(DebugDrawEncoder& dde, const physics_cylinder_shape& sh);

void draw(DebugDrawEncoder& dde, const physics_capsule_shape& sh);

void draw(DebugDrawEncoder& dde, const physics_box_shape& sh);

// void draw(DebugDrawEncoder& dde, const physics_plane_shape& sh);
// void draw(DebugDrawEncoder& dde, const physics_mesh_shape& sh);

void draw(DebugDrawEncoder& dde, const physics_compound_shape& sh);
void draw(DebugDrawEncoder& dde, const std::vector<physics_compound_shape>& sh);

} // namespace ace
