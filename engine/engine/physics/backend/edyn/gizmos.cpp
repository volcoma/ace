#include "gizmos.h"

#include <bx/math.h>
namespace ace
{

bx::Vec3 to_bx(edyn::vector3 v)
{
    return {float(v.x), float(v.y), float(v.z)};
}

bx::Quaternion to_bx(edyn::quaternion q)
{
    return {float(q.x), float(q.y), float(q.z), float(q.w)};
}

void draw(DebugDrawEncoder& dde, const edyn::sphere_shape& sh)
{
    bx::Sphere sphere;
    sphere.center = {0, 0, 0};
    sphere.radius = sh.radius;
    dde.draw(sphere);
}

void draw(DebugDrawEncoder& dde, const edyn::plane_shape& sh)
{
    auto center = sh.normal * sh.constant;
    dde.drawQuad(to_bx(-sh.normal), to_bx(center), 20);
}

void draw(DebugDrawEncoder& dde, const edyn::cylinder_shape& sh)
{
    auto axis = edyn::coordinate_axis_vector(sh.axis);
    dde.drawCylinder(to_bx(axis * -sh.half_length), to_bx(axis * sh.half_length), sh.radius);
}

void draw(DebugDrawEncoder& dde, const edyn::capsule_shape& sh)
{
    auto axis = edyn::coordinate_axis_vector(sh.axis);
    dde.drawCapsule(to_bx(axis * -sh.half_length), to_bx(axis * sh.half_length), sh.radius);
}

void draw(DebugDrawEncoder& dde, const edyn::box_shape& sh)
{
    auto aabb = bx::Aabb{to_bx(-sh.half_extents), to_bx(sh.half_extents)};
    dde.draw(aabb);
}

void assignVertexFrictionColor(DebugDrawEncoder& dde, edyn::triangle_mesh& trimesh, size_t edge_idx, size_t v_idx)
{
    auto friction = trimesh.get_vertex_friction(trimesh.get_edge_vertex_indices(edge_idx)[v_idx]);
    auto b = static_cast<uint32_t>(edyn::lerp(0xc0, 0x00, friction));
    auto g = static_cast<uint32_t>(edyn::lerp(0xc0, 0x00, friction));
    auto r = static_cast<uint32_t>(edyn::lerp(0xc0, 0xff, friction));
    dde.setColor(0xff000000 | (b << 16) | (g << 8) | r);
}

void draw(DebugDrawEncoder& dde, const edyn::mesh_shape& sh)
{
    dde.setWireframe(false);
    dde.push();

    auto& trimesh = sh.trimesh;

    for(size_t i = 0; i < trimesh->num_edges(); ++i)
    {
        dde.setColor(trimesh->is_boundary_edge(i) ? 0xff1081ea : 0xffc0c0c0);
        auto vertices = trimesh->get_edge_vertices(i);
        auto& v0 = vertices[0];
        auto& v1 = vertices[1];

        if(trimesh->has_per_vertex_friction())
        {
            assignVertexFrictionColor(dde, *trimesh, i, 0);
        }
        dde.moveTo(v0.x, v0.y, v0.z);

        if(trimesh->has_per_vertex_friction())
        {
            assignVertexFrictionColor(dde, *trimesh, i, 1);
        }
        dde.lineTo(v1.x, v1.y, v1.z);
    }

    dde.pop();
}

void draw(DebugDrawEncoder& dde, const edyn::paged_mesh_shape& sh)
{
    dde.setWireframe(false);
    sh.trimesh->visit_all_cached_edges(
        [&](auto mesh_idx, auto edge_idx)
        {
            auto trimesh = sh.trimesh->get_submesh(mesh_idx);
            dde.setColor(trimesh->is_boundary_edge(edge_idx) ? 0xff1081ea : 0xffc0c0c0);
            auto vertices = trimesh->get_edge_vertices(edge_idx);
            auto& v0 = vertices[0];
            auto& v1 = vertices[1];

            if(trimesh->has_per_vertex_friction())
            {
                assignVertexFrictionColor(dde, *trimesh, edge_idx, 0);
            }
            dde.moveTo(v0.x, v0.y, v0.z);

            if(trimesh->has_per_vertex_friction())
            {
                assignVertexFrictionColor(dde, *trimesh, edge_idx, 1);
            }
            dde.lineTo(v1.x, v1.y, v1.z);
        });
}

void draw(DebugDrawEncoder& dde, const edyn::polyhedron_shape& sh)
{
    for(size_t i = 0; i < sh.mesh->num_faces(); ++i)
    {
        auto first = sh.mesh->faces[i * 2];
        auto count = sh.mesh->faces[i * 2 + 1];

        auto i0 = sh.mesh->indices[first];
        auto& v0 = sh.mesh->vertices[i0];

        for(size_t j = 1; j < size_t(count - 1); ++j)
        {
            auto i1 = sh.mesh->indices[first + j];
            auto i2 = sh.mesh->indices[first + j + 1];
            auto& v1 = sh.mesh->vertices[i1];
            auto& v2 = sh.mesh->vertices[i2];

            auto tri = bx::Triangle{bx::Vec3(v0.x, v0.y, v0.z), bx::Vec3(v1.x, v1.y, v1.z), bx::Vec3(v2.x, v2.y, v2.z)};
            dde.draw(tri);
        }
    }
}

void draw(DebugDrawEncoder& dde, const edyn::compound_shape& sh)
{
    for(auto& node : sh.nodes)
    {
        auto pos = node.position;
        auto orn = node.orientation;

        auto bxquat = to_bx(orn);
        float rot[16];
        bx::mtxFromQuaternion(rot, bxquat);
        float rotT[16];
        bx::mtxTranspose(rotT, rot);
        float trans[16];
        bx::mtxTranslate(trans, pos.x, pos.y, pos.z);

        float mtx[16];
        bx::mtxMul(mtx, rotT, trans);

        dde.pushTransform(mtx);

        std::visit(
            [&](auto&& s)
            {
                draw(dde, s);
            },
            node.shape_var);

        dde.popTransform();
    }
}

void draw(DebugDrawEncoder& dde, entt::entity entity, const edyn::contact_manifold& manifold, const entt::registry& reg)
{
    // auto &posA = reg.get<edyn::position>(con.body[0]);
    // auto &ornA = reg.get<edyn::orientation>(con.body[0]);
    auto posB = edyn::get_rigidbody_origin(reg, manifold.body[1]);
    auto ornB = reg.get<edyn::orientation>(manifold.body[1]);

    manifold.each_point(
        [&](auto& cp)
        {
            auto pB = edyn::to_world_space(cp.pivotB, posB, ornB);
            auto tip = pB + cp.normal * 0.1;

            dde.push();

            dde.setColor(0xff3300fe);
            dde.moveTo(pB.x, pB.y, pB.z);
            dde.lineTo(tip.x, tip.y, tip.z);

            dde.pop();
        });
}

void draw(DebugDrawEncoder& dde, entt::entity entity, const edyn::contact_constraint& con, const entt::registry& reg)
{
    auto& manifold = reg.get<edyn::contact_manifold>(entity);
    draw(dde, entity, manifold, reg);
}

void draw(DebugDrawEncoder& dde, entt::entity, const edyn::point_constraint&, const entt::registry&)
{
}

void draw(DebugDrawEncoder& dde, entt::entity, const edyn::cvjoint_constraint&, const entt::registry&)
{
}

auto get_transforms(const entt::registry& reg, const edyn::constraint_base& con)
{
    auto posA = reg.any_of<edyn::present_position>(con.body[0]) ? edyn::get_rigidbody_present_origin(reg, con.body[0])
                                                                : edyn::get_rigidbody_origin(reg, con.body[0]);

    auto ornA = reg.any_of<edyn::present_orientation>(con.body[0])
                    ? static_cast<edyn::quaternion>(reg.get<edyn::present_orientation>(con.body[0]))
                    : static_cast<edyn::quaternion>(reg.get<edyn::orientation>(con.body[0]));

    auto posB = reg.any_of<edyn::present_position>(con.body[1]) ? edyn::get_rigidbody_present_origin(reg, con.body[1])
                                                                : edyn::get_rigidbody_origin(reg, con.body[1]);

    auto ornB = reg.any_of<edyn::present_orientation>(con.body[1])
                    ? static_cast<edyn::quaternion>(reg.get<edyn::present_orientation>(con.body[1]))
                    : static_cast<edyn::quaternion>(reg.get<edyn::orientation>(con.body[1]));

    return std::make_tuple(posA, ornA, posB, ornB);
}

void draw(DebugDrawEncoder& dde, entt::entity entity, const edyn::distance_constraint& con, const entt::registry& reg)
{
    auto [posA, ornA, posB, ornB] = get_transforms(reg, con);

    auto pA = edyn::to_world_space(con.pivot[0], posA, ornA);
    auto pB = edyn::to_world_space(con.pivot[1], posB, ornB);

    dde.push();

    dde.setColor(0xff0000fe);
    dde.moveTo(pA.x, pA.y, pA.z);
    dde.lineTo(pB.x, pB.y, pB.z);

    dde.pop();
}

void draw(DebugDrawEncoder& dde,
          entt::entity entity,
          const edyn::soft_distance_constraint& con,
          const entt::registry& reg)
{
    auto [posA, ornA, posB, ornB] = get_transforms(reg, con);

    auto pA = edyn::to_world_space(con.pivot[0], posA, ornA);
    auto pB = edyn::to_world_space(con.pivot[1], posB, ornB);

    dde.push();

    dde.setColor(0xff0000fe);
    dde.moveTo(pA.x, pA.y, pA.z);
    dde.lineTo(pB.x, pB.y, pB.z);

    dde.pop();
}

void draw(DebugDrawEncoder& dde, entt::entity entity, const edyn::hinge_constraint& con, const entt::registry& reg)
{
    auto [posA, ornA, posB, ornB] = get_transforms(reg, con);

    auto pA = edyn::to_world_space(con.pivot[0], posA, ornA);
    auto pB = edyn::to_world_space(con.pivot[1], posB, ornB);
    auto axisA = edyn::rotate(ornA, con.frame[0].column(0));
    auto axisB = edyn::rotate(ornB, con.frame[1].column(0));

    auto pA1 = pA + axisA * 0.2;
    auto pB1 = pB + axisB * 0.2;

    dde.push();

    dde.setColor(0xff0000fe);

    dde.moveTo(pA.x, pA.y, pA.z);
    dde.lineTo(pA1.x, pA1.y, pA1.z);

    dde.moveTo(pB.x, pB.y, pB.z);
    dde.lineTo(pB1.x, pB1.y, pB1.z);

    dde.pop();
}

void draw(DebugDrawEncoder& dde, entt::entity entity, const edyn::generic_constraint& con, const entt::registry& reg)
{
}

void draw(DebugDrawEncoder& dde, entt::entity entity, const edyn::null_constraint& con, const entt::registry& reg)
{
}

void draw(DebugDrawEncoder& dde, entt::entity entity, const edyn::gravity_constraint& con, const entt::registry& reg)
{
}

void draw(DebugDrawEncoder& dde, entt::entity entity, const edyn::cone_constraint& con, const entt::registry& reg)
{
    float scale = 1.0f;

    auto [posA, ornA, posB, ornB] = get_transforms(reg, con);

    float rot[16];
    float rotT[16];
    float trans[16];
    float mtx[16];

    bx::mtxFromQuaternion(rot, to_bx(ornA));
    bx::mtxTranspose(rotT, rot);
    bx::mtxTranslate(trans, posA.x, posA.y, posA.z);
    bx::mtxMul(mtx, rotT, trans);

    dde.pushTransform(mtx);

    float frame[16] = {(float)con.frame.row[0].x,
                       (float)con.frame.row[0].y,
                       (float)con.frame.row[0].z,
                       0.f,
                       (float)con.frame.row[1].x,
                       (float)con.frame.row[1].y,
                       (float)con.frame.row[1].z,
                       0.f,
                       (float)con.frame.row[2].x,
                       (float)con.frame.row[2].y,
                       (float)con.frame.row[2].z,
                       0.f,
                       0.f,
                       0.f,
                       0.f,
                       1.f};
    bx::mtxTranspose(rotT, frame);
    bx::mtxTranslate(trans, con.pivot[0].x, con.pivot[0].y, con.pivot[0].z);
    bx::mtxMul(mtx, rotT, trans);

    dde.pushTransform(mtx);

    auto radius0 = std::sin(std::atan(con.span_tan[0]));
    auto radius1 = std::sin(std::atan(con.span_tan[1]));

    auto num_points = 36;

    for(auto i = 0; i < num_points + 1; ++i)
    {
        auto angle = (float)i / (float)num_points * edyn::pi2;
        auto cos = std::cos(angle);
        auto sin = std::sin(angle);
        auto p = edyn::vector3{};
        p.y = cos * radius0;
        p.z = sin * radius1;
        p.x = std::sqrt(1 - (p.y * p.y + p.z * p.z));
        p *= scale;

        if(i == 0)
        {
            dde.moveTo(to_bx(p));
        }
        else
        {
            dde.lineTo(to_bx(p));
        }

        if(i % (num_points / 8) == 0)
        {
            dde.moveTo(0, 0, 0);
            dde.lineTo(to_bx(p));
        }
    }

    dde.popTransform();
    dde.popTransform();
}

void drawRaycastResult(DebugDrawEncoder& dde,
                       edyn::box_shape& box,
                       const edyn::shape_raycast_result& result,
                       edyn::vector3 rayPos,
                       edyn::vector3 rayDir,
                       edyn::vector3 pos,
                       edyn::quaternion orn)
{
    auto rayPosLocal = edyn::to_object_space(rayPos, pos, orn);
    auto rayDirLocal = edyn::rotate(edyn::conjugate(orn), rayDir);
    auto intersection = rayPosLocal + rayDirLocal * result.fraction;

    auto face_idx = std::get<edyn::box_raycast_info>(result.info_var).face_index;
    auto [feature, feature_idx] = box.get_closest_feature_on_face(face_idx, intersection, 0.01);

    switch(feature)
    {
        case edyn::box_feature::vertex:
        {
            auto v = box.get_vertex(feature_idx);
            auto normal = edyn::normalize(rayDirLocal);
            dde.drawQuad(to_bx(normal), to_bx(v), 0.015f);
            break;
        }
        case edyn::box_feature::edge:
        {
            auto v = box.get_edge(feature_idx);
            dde.moveTo(v[0].x, v[0].y, v[0].z);
            dde.lineTo(v[1].x, v[1].y, v[1].z);
            break;
        }
        case edyn::box_feature::face:
        {
            auto v = box.get_face(feature_idx);
            dde.moveTo(v[0].x, v[0].y, v[0].z);
            for(auto i = 0; i < 4; ++i)
            {
                auto j = (i + 1) % 4;
                dde.lineTo(v[j].x, v[j].y, v[j].z);
            }
        }
    }
}

void drawRaycastResult(DebugDrawEncoder& dde,
                       edyn::cylinder_shape& cylinder,
                       const edyn::shape_raycast_result& result,
                       edyn::vector3 rayPos,
                       edyn::vector3 rayDir,
                       edyn::vector3 pos,
                       edyn::quaternion orn)
{
    auto rayPosLocal = edyn::to_object_space(rayPos, pos, orn);
    auto rayDirLocal = edyn::rotate(edyn::conjugate(orn), rayDir);
    auto intersection = rayPosLocal + rayDirLocal * result.fraction;

    auto info = std::get<edyn::cylinder_raycast_info>(result.info_var);
    const auto axis = edyn::coordinate_axis_vector(cylinder.axis);
    auto vertices = std::array<edyn::vector3, 2>{axis * cylinder.half_length, axis * -cylinder.half_length};

    auto feature = info.feature;
    auto feature_index = info.face_index;
    auto tolerance = edyn::scalar(0.01);

    // Set feature as cap_edge if intersection is close to it.
    if(info.feature == edyn::cylinder_feature::face)
    {
        if(edyn::distance_sqr(intersection, vertices[info.face_index]) > edyn::square(cylinder.radius - tolerance))
        {
            feature = edyn::cylinder_feature::cap_edge;
        }
    }
    else if(info.feature == edyn::cylinder_feature::side_edge)
    {
        auto proj = edyn::dot(intersection, axis);
        if(std::abs(proj) > cylinder.half_length - tolerance)
        {
            feature = edyn::cylinder_feature::cap_edge;
            feature_index = proj > 0 ? 0 : 1;
        }
    }

    switch(feature)
    {
        case edyn::cylinder_feature::cap_edge:
        {
            auto center = vertices[feature_index];
            dde.drawCircle(to_bx(axis), to_bx(center), cylinder.radius);
            break;
        }
        case edyn::cylinder_feature::face:
        {
            auto from = vertices[feature_index];
            auto to = from + axis * 0.001f * (feature_index == 0 ? 1 : -1);
            dde.drawCylinder(to_bx(from), to_bx(to), cylinder.radius);
            break;
        }
        case edyn::cylinder_feature::side_edge:
        {
            auto p0 = edyn::project_plane(intersection, vertices[0], axis);
            auto p1 = edyn::project_plane(intersection, vertices[1], axis);
            dde.moveTo(p0.x, p0.y, p0.z);
            dde.lineTo(p1.x, p1.y, p1.z);
        }
    }
}

void drawRaycastResult(DebugDrawEncoder& dde,
                       edyn::sphere_shape& sphere,
                       const edyn::shape_raycast_result& result,
                       edyn::vector3 rayPos,
                       edyn::vector3 rayDir,
                       edyn::vector3 pos,
                       edyn::quaternion orn)
{
    auto axis = edyn::rotate(edyn::conjugate(orn), edyn::normalize(rayDir));
    dde.drawCircle(to_bx(axis), {0, 0, 0}, sphere.radius);
}

void drawRaycastResult(DebugDrawEncoder& dde,
                       edyn::capsule_shape& capsule,
                       const edyn::shape_raycast_result& result,
                       edyn::vector3 rayPos,
                       edyn::vector3 rayDir,
                       edyn::vector3 pos,
                       edyn::quaternion orn)
{
    const auto axis = edyn::coordinate_axis_vector(capsule.axis);
    auto vertices = std::array<edyn::vector3, 2>{axis * capsule.half_length, axis * -capsule.half_length};
    auto info = std::get<edyn::capsule_raycast_info>(result.info_var);

    switch(info.feature)
    {
        case edyn::capsule_feature::hemisphere:
        {
            auto center = vertices[info.hemisphere_index];
            dde.drawCircle(to_bx(axis), to_bx(center), capsule.radius);
            break;
        }
        case edyn::capsule_feature::side:
        {
            auto rayPosLocal = edyn::to_object_space(rayPos, pos, orn);
            auto rayDirLocal = edyn::rotate(edyn::conjugate(orn), rayDir);
            auto intersection = rayPosLocal + rayDirLocal * result.fraction;
            auto p0 = edyn::project_plane(intersection, vertices[0], axis);
            auto p1 = edyn::project_plane(intersection, vertices[1], axis);
            dde.moveTo(p0.x, p0.y, p0.z);
            dde.lineTo(p1.x, p1.y, p1.z);
        }
    }
}

void drawRaycastResult(DebugDrawEncoder& dde,
                       edyn::polyhedron_shape& poly,
                       const edyn::shape_raycast_result& result,
                       edyn::vector3 rayPos,
                       edyn::vector3 rayDir,
                       edyn::vector3 pos,
                       edyn::quaternion orn)
{
    auto rayPosLocal = edyn::to_object_space(rayPos, pos, orn);
    auto rayDirLocal = edyn::rotate(edyn::conjugate(orn), rayDir);
    auto intersection = rayPosLocal + rayDirLocal * result.fraction;

    auto face_idx = std::get<edyn::polyhedron_raycast_info>(result.info_var).face_index;
    auto tolerance = edyn::scalar(0.01);
    auto tolerance_sqr = tolerance * tolerance;
    auto num_vertices = poly.mesh->face_vertex_count(face_idx);

    for(unsigned i = 0; i < num_vertices; ++i)
    {
        auto v_idx = poly.mesh->face_vertex_index(face_idx, i);
        auto v = poly.mesh->vertices[v_idx];

        if(edyn::distance_sqr(v, intersection) < tolerance_sqr)
        {
            auto normal = edyn::normalize(rayDirLocal);
            dde.drawQuad(to_bx(normal), to_bx(v), 0.015f);
            return;
        }
    }

    for(unsigned i = 0; i < num_vertices; ++i)
    {
        auto v0_idx = poly.mesh->face_vertex_index(face_idx, i);
        auto v1_idx = poly.mesh->face_vertex_index(face_idx, (i + 1) % num_vertices);
        auto v0 = poly.mesh->vertices[v0_idx];
        auto v1 = poly.mesh->vertices[v1_idx];

        if(edyn::distance_sqr_line(v0, v1 - v0, intersection) < tolerance_sqr)
        {
            dde.moveTo(v0.x, v0.y, v0.z);
            dde.lineTo(v1.x, v1.y, v1.z);
            return;
        }
    }

    for(size_t i = 0; i < num_vertices; ++i)
    {
        auto v_idx = poly.mesh->face_vertex_index(face_idx, i);
        auto v = poly.mesh->vertices[v_idx];
        if(i == 0)
        {
            dde.moveTo(v.x, v.y, v.z);
        }
        else
        {
            dde.lineTo(v.x, v.y, v.z);
        }
    }
    dde.close();
}

void drawRaycastResult(DebugDrawEncoder& dde,
                       edyn::compound_shape& compound,
                       const edyn::shape_raycast_result& result,
                       edyn::vector3 rayPos,
                       edyn::vector3 rayDir,
                       edyn::vector3 pos,
                       edyn::quaternion orn)
{
    auto rayPosLocal = edyn::to_object_space(rayPos, pos, orn);
    auto rayDirLocal = edyn::rotate(edyn::conjugate(orn), rayDir);
    auto info = std::get<edyn::compound_raycast_info>(result.info_var);
    auto& node = compound.nodes[info.child_index];

    auto bxquat = to_bx(node.orientation);

    float rot[16];
    bx::mtxFromQuaternion(rot, bxquat);
    float rotT[16];
    bx::mtxTranspose(rotT, rot);
    float trans[16];
    bx::mtxTranslate(trans, node.position.x, node.position.y, node.position.z);

    float mtx[16];
    bx::mtxMul(mtx, rotT, trans);

    dde.pushTransform(mtx);

    std::visit(
        [&](auto&& shape)
        {
            auto child_result = edyn::shape_raycast_result{result.fraction, result.normal};

            std::visit(
                [&](auto&& child_info)
                {
                    child_result.info_var = child_info;
                },
                info.child_info_var);

            drawRaycastResult(dde, shape, child_result, rayPosLocal, rayDirLocal, node.position, node.orientation);
        },
        node.shape_var);

    dde.popTransform();
}

void drawRaycastResult(DebugDrawEncoder& dde,
                       edyn::plane_shape& plane,
                       const edyn::shape_raycast_result& result,
                       edyn::vector3 rayPos,
                       edyn::vector3 rayDir,
                       edyn::vector3 pos,
                       edyn::quaternion orn)
{
}

void drawRaycastResult(DebugDrawEncoder& dde,
                       edyn::mesh_shape& mesh,
                       const edyn::shape_raycast_result& result,
                       edyn::vector3 rayPos,
                       edyn::vector3 rayDir,
                       edyn::vector3 pos,
                       edyn::quaternion orn)
{
    auto tri_idx = std::get<edyn::mesh_raycast_info>(result.info_var).triangle_index;
    auto vertices = mesh.trimesh->get_triangle_vertices(tri_idx);
    auto tri = bx::Triangle{bx::Vec3(vertices[0].x, vertices[0].y, vertices[0].z),
                            bx::Vec3(vertices[1].x, vertices[1].y, vertices[1].z),
                            bx::Vec3(vertices[2].x, vertices[2].y, vertices[2].z)};
    dde.draw(tri);
}

void drawRaycastResult(DebugDrawEncoder& dde,
                       edyn::paged_mesh_shape& paged_mesh,
                       const edyn::shape_raycast_result& result,
                       edyn::vector3 rayPos,
                       edyn::vector3 rayDir,
                       edyn::vector3 pos,
                       edyn::quaternion orn)
{
    auto info = std::get<edyn::paged_mesh_raycast_info>(result.info_var);
    auto vertices = paged_mesh.trimesh->get_triangle_vertices(info.submesh_index, info.triangle_index);
    auto tri = bx::Triangle{bx::Vec3(vertices[0].x, vertices[0].y, vertices[0].z),
                            bx::Vec3(vertices[1].x, vertices[1].y, vertices[1].z),
                            bx::Vec3(vertices[2].x, vertices[2].y, vertices[2].z)};
    dde.draw(tri);
}

} // namespace ace
