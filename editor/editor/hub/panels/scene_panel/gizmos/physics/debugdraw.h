#pragma once

#include <edyn/edyn.hpp>
#include <edyn/shapes/polyhedron_shape.hpp>
#include <graphics/debugdraw.h>

namespace ace
{

bx::Vec3 to_bx(edyn::vector3 v);
bx::Quaternion to_bx(edyn::quaternion q);

void draw(DebugDrawEncoder& dde, const edyn::sphere_shape& sh);

void draw(DebugDrawEncoder& dde, const edyn::plane_shape& sh);

void draw(DebugDrawEncoder& dde, const edyn::cylinder_shape& sh);

void draw(DebugDrawEncoder& dde, const edyn::capsule_shape& sh);

void draw(DebugDrawEncoder& dde, const edyn::box_shape& sh);

void draw(DebugDrawEncoder& dde, const edyn::mesh_shape& sh);
void draw(DebugDrawEncoder& dde, const edyn::paged_mesh_shape& sh);
void draw(DebugDrawEncoder& dde, const edyn::polyhedron_shape& sh);
void draw(DebugDrawEncoder& dde, const edyn::compound_shape& sh);

void draw(DebugDrawEncoder& dde, entt::entity, const edyn::contact_constraint&, const entt::registry&);

void draw(DebugDrawEncoder& dde, entt::entity, const edyn::point_constraint&, const entt::registry&);
void draw(DebugDrawEncoder& dde, entt::entity, const edyn::cvjoint_constraint&, const entt::registry&);

void draw(DebugDrawEncoder& dde, entt::entity, const edyn::cone_constraint&, const entt::registry&);

void draw(DebugDrawEncoder& dde, entt::entity, const edyn::distance_constraint&, const entt::registry&);
void draw(DebugDrawEncoder& dde, entt::entity, const edyn::soft_distance_constraint&, const entt::registry&);

void draw(DebugDrawEncoder& dde, entt::entity entity, const edyn::hinge_constraint& con, const entt::registry& reg);

void draw(DebugDrawEncoder& dde, entt::entity entity, const edyn::generic_constraint& con, const entt::registry& reg);

void draw(DebugDrawEncoder& dde, entt::entity entity, const edyn::null_constraint& con, const entt::registry& reg);

void draw(DebugDrawEncoder& dde, entt::entity entity, const edyn::gravity_constraint& con, const entt::registry& reg);

void draw(DebugDrawEncoder& dde,
          entt::entity entity,
          const edyn::contact_manifold& manifold,
          const entt::registry& reg);

void drawRaycastResult(DebugDrawEncoder& dde,
                       edyn::box_shape& box,
                       const edyn::shape_raycast_result& result,
                       edyn::vector3 rayPos,
                       edyn::vector3 rayDir,
                       edyn::vector3 pos,
                       edyn::quaternion orn);

void drawRaycastResult(DebugDrawEncoder& dde,
                       edyn::cylinder_shape& cylinder,
                       const edyn::shape_raycast_result& result,
                       edyn::vector3 rayPos,
                       edyn::vector3 rayDir,
                       edyn::vector3 pos,
                       edyn::quaternion orn);

void drawRaycastResult(DebugDrawEncoder& dde,
                       edyn::sphere_shape& sphere,
                       const edyn::shape_raycast_result& result,
                       edyn::vector3 rayPos,
                       edyn::vector3 rayDir,
                       edyn::vector3 pos,
                       edyn::quaternion orn);

void drawRaycastResult(DebugDrawEncoder& dde,
                       edyn::capsule_shape& capsule,
                       const edyn::shape_raycast_result& result,
                       edyn::vector3 rayPos,
                       edyn::vector3 rayDir,
                       edyn::vector3 pos,
                       edyn::quaternion orn);

void drawRaycastResult(DebugDrawEncoder& dde,
                       edyn::polyhedron_shape& poly,
                       const edyn::shape_raycast_result& result,
                       edyn::vector3 rayPos,
                       edyn::vector3 rayDir,
                       edyn::vector3 pos,
                       edyn::quaternion orn);

void drawRaycastResult(DebugDrawEncoder& dde,
                       edyn::compound_shape& compound,
                       const edyn::shape_raycast_result& result,
                       edyn::vector3 rayPos,
                       edyn::vector3 rayDir,
                       edyn::vector3 pos,
                       edyn::quaternion orn);

void drawRaycastResult(DebugDrawEncoder& dde,
                       edyn::plane_shape& plane,
                       const edyn::shape_raycast_result& result,
                       edyn::vector3 rayPos,
                       edyn::vector3 rayDir,
                       edyn::vector3 pos,
                       edyn::quaternion orn);

void drawRaycastResult(DebugDrawEncoder& dde,
                       edyn::mesh_shape& mesh,
                       const edyn::shape_raycast_result& result,
                       edyn::vector3 rayPos,
                       edyn::vector3 rayDir,
                       edyn::vector3 pos,
                       edyn::quaternion orn);

void drawRaycastResult(DebugDrawEncoder& dde,
                       edyn::paged_mesh_shape& paged_mesh,
                       const edyn::shape_raycast_result& result,
                       edyn::vector3 rayPos,
                       edyn::vector3 rayDir,
                       edyn::vector3 pos,
                       edyn::quaternion orn);

} // namespace ace
