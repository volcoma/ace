#include "gizmos.h"

#include <bx/math.h>
namespace ace
{

auto to_bx(const glm::vec3& data) -> bx::Vec3
{
    return {data.x, data.y, data.z};
}

void draw(DebugDrawEncoder& dde, const physics_sphere_shape& sh)
{
    bx::Sphere sphere;
    sphere.center = to_bx(sh.center);
    sphere.radius = sh.radius;
    dde.draw(sphere);
}

// void draw(DebugDrawEncoder& dde, const physics_plane_shape& sh)
// {
//     auto center = sh.normal * sh.constant;
//     dde.drawQuad(to_bx(-sh.normal), to_bx(center), 20);
// }

void draw(DebugDrawEncoder& dde, const physics_cylinder_shape& sh)
{
    math::vec3 axis{0, 1, 0};
    dde.drawCylinder(to_bx(sh.center + axis * -sh.length * 0.5f),
                     to_bx(sh.center + axis * sh.length * 0.5f),
                     sh.radius);
}

void draw(DebugDrawEncoder& dde, const physics_capsule_shape& sh)
{
    // auto axis = edyn::coordinate_axis_vector(sh.axis);
    math::vec3 axis{0, 1, 0};
    dde.drawCapsule(to_bx(sh.center + axis * -sh.length * 0.5f), to_bx(sh.center + axis * sh.length * 0.5f), sh.radius);
}

void draw(DebugDrawEncoder& dde, const physics_box_shape& sh)
{
    auto aabb = bx::Aabb{to_bx(sh.center - sh.extends * 0.5f), to_bx(sh.center + sh.extends * 0.5f)};
    dde.draw(aabb);
}

// void draw(DebugDrawEncoder& dde, const physics_mesh_shape& sh)
// {

// }

void draw(DebugDrawEncoder& dde, const physics_compound_shape& sh)
{
    hpp::visit(
        [&](auto&& s)
        {
            draw(dde, s);
        },
        sh.shape);
}

void draw(DebugDrawEncoder& dde, const std::vector<physics_compound_shape>& shapes)
{
    for(auto& shape : shapes)
    {
        draw(dde, shape);
    }
}


} // namespace ace
