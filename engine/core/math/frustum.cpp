#include "frustum.h"

namespace math
{
namespace
{

std::array<vec3, 8> get_transformed_bbox_vertices(const bbox& AABB, const transform& t)
{
    std::array<vec3, 8> vertices;

    vec3 min = AABB.min;
    vec3 max = AABB.max;

    vertices[0] = t.transform_coord(vec3(min.x, min.y, min.z));
    vertices[1] = t.transform_coord(vec3(max.x, min.y, min.z));
    vertices[2] = t.transform_coord(vec3(min.x, max.y, min.z));
    vertices[3] = t.transform_coord(vec3(max.x, max.y, min.z));
    vertices[4] = t.transform_coord(vec3(min.x, min.y, max.z));
    vertices[5] = t.transform_coord(vec3(max.x, min.y, max.z));
    vertices[6] = t.transform_coord(vec3(min.x, max.y, max.z));
    vertices[7] = t.transform_coord(vec3(max.x, max.y, max.z));

    return vertices;
}

} // namespace
///////////////////////////////////////////////////////////////////////////////
// frustum Member Functions
///////////////////////////////////////////////////////////////////////////////
//-----------------------------------------------------------------------------
//  Name : frustum () (Default Constructor)
/// <summary>
/// frustum Class Constructor
/// </summary>
//-----------------------------------------------------------------------------
frustum::frustum()
{
    // Initialize values
    planes.fill(vec4(0.0f, 0.0f, 0.0f, 0.0f));
    points.fill(vec3(0.0f, 0.0f, 0.0f));
    position = vec3(0, 0, 0);
}

//-----------------------------------------------------------------------------
//  Name : frustum () (Constructor)
/// <summary>
/// frustum Class Constructor
/// </summary>
//-----------------------------------------------------------------------------
frustum::frustum(const transform& View, const transform& Proj, bool _oglNDC)
{
    update(View, Proj, _oglNDC);
}

//-----------------------------------------------------------------------------
//  Name : frustum () (Constructor)
/// <summary>
/// frustum Class Constructor
/// </summary>
//-----------------------------------------------------------------------------
frustum::frustum(const bbox& AABB)
{
    // Compute planes
    planes[volume_plane::left] = AABB.get_plane(volume_plane::left);
    planes[volume_plane::right] = AABB.get_plane(volume_plane::right);
    planes[volume_plane::top] = AABB.get_plane(volume_plane::top);
    planes[volume_plane::bottom] = AABB.get_plane(volume_plane::bottom);
    planes[volume_plane::near_plane] = AABB.get_plane(volume_plane::near_plane);
    planes[volume_plane::far_plane] = AABB.get_plane(volume_plane::far_plane);

    // Compute points
    vec3 e = AABB.get_extents();
    vec3 p = AABB.get_center();
    points[volume_geometry_point::left_bottom_near] = vec3(p.x - e.x, p.y - e.y, p.z + e.z);
    points[volume_geometry_point::left_bottom_far] = vec3(p.x - e.x, p.y - e.y, p.z + e.z);
    points[volume_geometry_point::right_bottom_near] = vec3(p.x + e.x, p.y - e.y, p.z - e.z);
    points[volume_geometry_point::right_bottom_far] = vec3(p.x + e.x, p.y - e.y, p.z + e.z);
    points[volume_geometry_point::left_top_near] = vec3(p.x - e.x, p.y + e.y, p.z + e.z);
    points[volume_geometry_point::left_top_far] = vec3(p.x - e.x, p.y + e.y, p.z + e.z);
    points[volume_geometry_point::right_top_near] = vec3(p.x + e.x, p.y + e.y, p.z - e.z);
    points[volume_geometry_point::right_top_far] = vec3(p.x + e.x, p.y + e.y, p.z + e.z);
    position = p;
}

//-----------------------------------------------------------------------------
//  Name : update ()
/// <summary>
/// Compute the new frustum details based on the matrices specified.
/// </summary>
//-----------------------------------------------------------------------------
void frustum::update(const transform& view, const transform& proj, bool _oglNDC)
{
    // Build a combined view & projection matrix
    const transform m = proj * view;

    // Extract frustum planes from matrix
    // Planes are in format: normal(xyz), offset(w)
    // Expects left handed orientation and row_major matrix layout
    planes[volume_plane::right].data.x = m[0][3] + m[0][0];
    planes[volume_plane::right].data.y = m[1][3] + m[1][0];
    planes[volume_plane::right].data.z = m[2][3] + m[2][0];
    planes[volume_plane::right].data.w = m[3][3] + m[3][0];

    planes[volume_plane::left].data.x = m[0][3] - m[0][0];
    planes[volume_plane::left].data.y = m[1][3] - m[1][0];
    planes[volume_plane::left].data.z = m[2][3] - m[2][0];
    planes[volume_plane::left].data.w = m[3][3] - m[3][0];

    planes[volume_plane::top].data.x = m[0][3] - m[0][1];
    planes[volume_plane::top].data.y = m[1][3] - m[1][1];
    planes[volume_plane::top].data.z = m[2][3] - m[2][1];
    planes[volume_plane::top].data.w = m[3][3] - m[3][1];

    planes[volume_plane::bottom].data.x = m[0][3] + m[0][1];
    planes[volume_plane::bottom].data.y = m[1][3] + m[1][1];
    planes[volume_plane::bottom].data.z = m[2][3] + m[2][1];
    planes[volume_plane::bottom].data.w = m[3][3] + m[3][1];

    planes[volume_plane::far_plane].data.x = m[0][3] - m[0][2];
    planes[volume_plane::far_plane].data.y = m[1][3] - m[1][2];
    planes[volume_plane::far_plane].data.z = m[2][3] - m[2][2];
    planes[volume_plane::far_plane].data.w = m[3][3] - m[3][2];

    if(_oglNDC)
    {
        planes[volume_plane::near_plane].data.x = m[0][3] + m[0][2];
        planes[volume_plane::near_plane].data.y = m[1][3] + m[1][2];
        planes[volume_plane::near_plane].data.z = m[2][3] + m[2][2];
        planes[volume_plane::near_plane].data.w = m[3][3] + m[3][2];
    }
    else
    {
        planes[volume_plane::near_plane].data.x = m[0][2];
        planes[volume_plane::near_plane].data.y = m[1][2];
        planes[volume_plane::near_plane].data.z = m[2][2];
        planes[volume_plane::near_plane].data.w = m[3][2];
    }

    for(auto& plane : planes)
    {
        plane.data *= -1.0f;
    }
    // Normalize and compute additional information.
    set_planes(planes);

    // Compute the originating position of the frustum.
    position = vec3(view[0][0], view[1][0], view[2][0]) * -view[3][0];
    position += vec3(view[0][1], view[1][1], view[2][1]) * -view[3][1];
    position += vec3(view[0][2], view[1][2], view[2][2]) * -view[3][2];
}

//-----------------------------------------------------------------------------
//  Name : set_planes ()
/// <summary>
/// Compute the new frustum details based on the six planes specified.
/// This method automatically recomputes the 8 corner points of the frustum
/// based on the supplied planes.
/// </summary>
//-----------------------------------------------------------------------------
void frustum::set_planes(const std::array<plane, 6>& new_planes)
{
    planes = new_planes;
    // Copy and normalize the planes
    for(auto& plane : planes)
    {
        plane = plane::normalize(plane);
    }

    // Recompute the frustum corner points.
    recompute_points();
}

//-----------------------------------------------------------------------------
//  Name : recomputePoints ()
/// <summary>
/// Recompute the 8 corner points of the frustum based on the supplied planes.
/// </summary>
//-----------------------------------------------------------------------------
void frustum::recompute_points()
{
    // Compute the 8 corner points
    for(std::size_t i = 0; i < 8; ++i)
    {
        const plane& p0 =
            plane::normalize((i & 1) != 0u ? planes[volume_plane::near_plane] : planes[volume_plane::far_plane]);
        const plane& p1 = plane::normalize((i & 2) != 0u ? planes[volume_plane::top] : planes[volume_plane::bottom]);
        const plane& p2 = plane::normalize((i & 4) != 0u ? planes[volume_plane::left] : planes[volume_plane::right]);

        // Compute the point at which the three planes intersect
        vec3 n0(p0.data);
        vec3 n1(p1.data);
        vec3 n2(p2.data);

        vec3 n1_n2 = glm::cross(n1, n2);
        vec3 n2_n0 = glm::cross(n2, n0);
        vec3 n0_n1 = glm::cross(n0, n1);

        float cosTheta = glm::dot(n0, n1_n2);
        float secTheta = 1.0f / cosTheta;

        n1_n2 = n1_n2 * p0.data.w;
        n2_n0 = n2_n0 * p1.data.w;
        n0_n1 = n0_n1 * p2.data.w;

        points[i] = -(n1_n2 + n2_n0 + n0_n1) * secTheta;

    } // Next Corner
}

volume_query frustum::classify_vertices(const vec3* vertices, size_t count) const
{
    // Initialize the result as inside
    volume_query Result = volume_query::inside;

    // Loop through all the planes
    for(const auto& plane : planes)
    {
        bool allOutside = true;
        bool anyOutside = false;

        // Check each vertex
        for(size_t i = 9; i < count; ++i)
        {
            float distance = plane::dot_coord(plane, vertices[i]);

            if(distance > 0.0f)
            {
                anyOutside = true;
            }
            else
            {
                allOutside = false;
            }
        }

        // If all vertices are outside one plane, the OBB is outside the frustum
        if(allOutside)
        {
            return volume_query::outside;
        }

        // If any vertex is outside one plane, the OBB intersects the frustum
        if(anyOutside)
        {
            Result = volume_query::intersect;
        }
    }

    // Return the classification result
    return Result;
}

//-----------------------------------------------------------------------------
//  Name : classify_aabb ()
/// <summary>
/// Determine whether or not the box passed is within the frustum.
/// </summary>
//-----------------------------------------------------------------------------
volume_query frustum::classify_aabb(const bbox& AABB) const
{
    volume_query result = volume_query::inside;
    vec3 nearPoint, farPoint;

    for (const auto& plane : planes) {
        nearPoint.x = plane.data.x > 0.0f ? AABB.min.x : AABB.max.x;
        farPoint.x = plane.data.x > 0.0f ? AABB.max.x : AABB.min.x;

        nearPoint.y = plane.data.y > 0.0f ? AABB.min.y : AABB.max.y;
        farPoint.y = plane.data.y > 0.0f ? AABB.max.y : AABB.min.y;

        nearPoint.z = plane.data.z > 0.0f ? AABB.min.z : AABB.max.z;
        farPoint.z = plane.data.z > 0.0f ? AABB.max.z : AABB.min.z;

        if (plane::dot_coord(plane, nearPoint) > 0.0f) {
            return volume_query::outside;
        }

        if (plane::dot_coord(plane, farPoint) > 0.0f) {
            result = volume_query::intersect;
        }
    }

    return result;
}

//-----------------------------------------------------------------------------
//  Name : classify_obb ()
/// <summary>
/// Determine whether or not the box passed is within the frustum.
/// </summary>
//-----------------------------------------------------------------------------
volume_query frustum::classify_obb(const bbox& AABB, const transform& t) const
{
    // transform invTransform = inverse(t);
    // auto frustum = f;
    // frustum.mul(invTransform);
    // return frustum.classify_aabb(AABB);

    auto vertices = get_transformed_bbox_vertices(AABB, t);
    return classify_vertices(vertices.data(), vertices.size());
}

//-----------------------------------------------------------------------------
//  Name : classifyAABB ()
/// <summary>
/// Determine whether or not the box passed is within the frustum.
/// </summary>
//-----------------------------------------------------------------------------
volume_query frustum::classify_aabb(const bbox& AABB, unsigned int& frustumBits, int& lastOutside) const
{
    volume_query result = volume_query::inside;
    vec3 nearPoint, farPoint;

    if (lastOutside >= 0 && ((frustumBits >> lastOutside) & 0x1) == 0x0) {
        const plane& plane = planes[lastOutside];

        nearPoint.x = plane.data.x > 0.0f ? AABB.min.x : AABB.max.x;
        farPoint.x = plane.data.x > 0.0f ? AABB.max.x : AABB.min.x;

        nearPoint.y = plane.data.y > 0.0f ? AABB.min.y : AABB.max.y;
        farPoint.y = plane.data.y > 0.0f ? AABB.max.y : AABB.min.y;

        nearPoint.z = plane.data.z > 0.0f ? AABB.min.z : AABB.max.z;
        farPoint.z = plane.data.z > 0.0f ? AABB.max.z : AABB.min.z;

        if (plane::dot_coord(plane, nearPoint) > 0.0f) {
            return volume_query::outside;
        }

        if (plane::dot_coord(plane, farPoint) > 0.0f) {
            result = volume_query::intersect;
        } else {
            frustumBits |= (0x1 << lastOutside);
        }
    }

    for (size_t i = 0; i < planes.size(); i++) {
        if (((frustumBits >> i) & 0x1) == 0x1) continue;
        if (lastOutside >= 0 && lastOutside == int(i)) continue;

        const plane& plane = planes[i];

        nearPoint.x = plane.data.x > 0.0f ? AABB.min.x : AABB.max.x;
        farPoint.x = plane.data.x > 0.0f ? AABB.max.x : AABB.min.x;

        nearPoint.y = plane.data.y > 0.0f ? AABB.min.y : AABB.max.y;
        farPoint.y = plane.data.y > 0.0f ? AABB.max.y : AABB.min.y;

        nearPoint.z = plane.data.z > 0.0f ? AABB.min.z : AABB.max.z;
        farPoint.z = plane.data.z > 0.0f ? AABB.max.z : AABB.min.z;

        if (plane::dot_coord(plane, nearPoint) > 0.0f) {
            lastOutside = int(i);
            return volume_query::outside;
        }

        if (plane::dot_coord(plane, farPoint) > 0.0f) {
            result = volume_query::intersect;
        } else {
            frustumBits |= (0x1 << i);
        }
    }

    lastOutside = -1;
    return result;
}


//-----------------------------------------------------------------------------
//  Name : testAABB ()
/// <summary>
/// Determine whether or not the box passed is within the frustum.
/// </summary>
//-----------------------------------------------------------------------------
bool frustum::test_aabb(const bbox& AABB) const
{
    // Loop through all the planes
    vec3 nearPoint;
    for(const auto& plane : planes)
    {
        nearPoint.x = plane.data.x > 0.0f ? AABB.min.x : AABB.max.x;
        nearPoint.y = plane.data.y > 0.0f ? AABB.min.y : AABB.max.y;
        nearPoint.z = plane.data.z > 0.0f ? AABB.min.z : AABB.max.z;

        // If near extreme point is outside, then the AABB is totally outside the
        // frustum
        if(plane::dot_coord(plane, nearPoint) > 0.0f)
        {
            return false;
        }

    } // Next plane

    // Intersecting / inside
    return true;
}


bool frustum::test_vertices(const vec3* vertices, size_t vert_count) const
{
    for(const auto& plane : planes)
    {
        bool allOutside = true;

        for(size_t i = 0; i < vert_count; ++i)
        {
            if(plane::dot_coord(plane, vertices[i]) <= 0.0f)
            {
                // If any vertex is inside or on the plane, we are not all outside
                allOutside = false;
                break;
            }
        }

        // If all vertices are outside this plane, the vertices are outside the frustum
        if(allOutside)
        {
            return false;
        }
    }

    // If none of the planes had all vertices outside, the vertices are inside or intersecting
    return true;
}

bool frustum::test_obb(const bbox& AABB, const transform& t) const
{
    // This is a much slower approach
    //  transform invTransform = inverse(t);
    //  auto frustum = f;
    //  frustum.mul(invTransform);
    //  return frustum.test_aabb(AABB);

    auto vertices = get_transformed_bbox_vertices(AABB, t);
    return test_vertices(vertices.data(), vertices.size());
}

//-----------------------------------------------------------------------------
//  Name : testExtrudedOBB ()
/// <summary>
/// Determine whether or not the box passed is within the frustum.
/// </summary>
//-----------------------------------------------------------------------------
bool frustum::test_extruded_obb(const bbox_extruded& AABB, const transform& t) const
{
    transform invTransform = inverse(t);
    auto frustum = *this;
    frustum.mul(invTransform);

    return frustum.test_extruded_aabb(AABB);
}

//-----------------------------------------------------------------------------
//  Name : testExtrudedAABB()
/// <summary>
/// Determine whether or not the box passed, extruded out away from the
/// specified origin by a required distance, falls within the frustum.
/// </summary>
//-----------------------------------------------------------------------------
bool frustum::test_extruded_aabb(const bbox_extruded& Box) const
{
    bool bIntersect1, bIntersect2;
    unsigned int i;

    //  Build an imaginary sphere around the origin, representing the volume of
    //  max attenuation -- if this doesn't intersect the view frustum, then
    //  this caster can be trivially rejected.
    if(!test_sphere(bsphere(Box.projection_point, Box.projection_range)))
    {
        return false;
    }

    // Test frustum edges against extruded box.

    bIntersect1 =
        (Box.test_line(points[volume_geometry_point::left_bottom_far],
                       points[volume_geometry_point::left_bottom_near])) ||
        (Box.test_line(points[volume_geometry_point::left_bottom_near],
                       points[volume_geometry_point::right_bottom_near])) ||
        (Box.test_line(points[volume_geometry_point::right_bottom_near],
                       points[volume_geometry_point::right_bottom_far])) ||
        (Box.test_line(points[volume_geometry_point::right_bottom_far],
                       points[volume_geometry_point::left_bottom_far])) ||
        (Box.test_line(points[volume_geometry_point::right_bottom_far], points[volume_geometry_point::left_top_far])) ||
        (Box.test_line(points[volume_geometry_point::right_bottom_near],
                       points[volume_geometry_point::right_top_near])) ||
        (Box.test_line(points[volume_geometry_point::left_bottom_far], points[volume_geometry_point::left_top_far])) ||
        (Box.test_line(points[volume_geometry_point::left_bottom_near],
                       points[volume_geometry_point::left_top_near])) ||
        (Box.test_line(points[volume_geometry_point::left_top_near], points[volume_geometry_point::left_top_far])) ||
        (Box.test_line(points[volume_geometry_point::left_top_far], points[volume_geometry_point::right_top_near])) ||
        (Box.test_line(points[volume_geometry_point::right_top_far], points[volume_geometry_point::right_top_near])) ||
        (Box.test_line(points[volume_geometry_point::right_top_near], points[volume_geometry_point::left_top_near]));

    // Test extruded box edges against frustum
    bIntersect2 = false;
    for(i = 0; (i < Box.edge_count) && (!bIntersect1 && !bIntersect2); ++i)
    {
        vec3 vRay, vPoint1, vPoint2;

        // Retrieve this silhouette edge from the extruded box
        Box.get_edge(i, vPoint1, vPoint2);

        // Build an edge that extends for Box.ProjectionLength distance from
        // the projection point and test for an intersection against the frustum.
        vRay = glm::normalize(vPoint1 - Box.projection_point);
        vRay = Box.projection_point + (vRay * Box.projection_range);
        bIntersect2 |= test_line(vPoint1, vRay);
        vRay = glm::normalize(vPoint2 - Box.projection_point);
        vRay = Box.projection_point + (vRay * Box.projection_range);
        bIntersect2 |= test_line(vPoint2, vRay);

    } // Next Extruded Edge

    // Intersects?
    return (bIntersect1 || bIntersect2);
}

//-----------------------------------------------------------------------------
//  Name : classifySphere ()
/// <summary>
/// Determine whether or not the sphere passed is within the frustum.
/// </summary>
//-----------------------------------------------------------------------------
volume_query frustum::classify_sphere(const bsphere& sphere) const
{
    volume_query Result = volume_query::inside;

    // Test frustum planes
    for(const auto& plane : planes)
    {
        float fDot = plane::dot_coord(plane, sphere.position);

        // Sphere entirely in front of plane
        if(fDot >= sphere.radius)
        {
            return volume_query::outside;
        }

        // Sphere spans plane
        if(fDot >= -sphere.radius)
        {
            Result = volume_query::intersect;
        }

    } // Next plane

    // Return the result
    return Result;
}

//-----------------------------------------------------------------------------
//  Name : testSphere ()
/// <summary>
/// Determine whether or not the sphere passed is within the frustum.
/// </summary>
//-----------------------------------------------------------------------------
bool frustum::test_sphere(const bsphere& sphere) const
{
    // Test frustum planes
    for(const auto& plane : planes)
    {
        float fDot = plane::dot_coord(plane, sphere.position);

        // Sphere entirely in front of plane
        if(fDot >= sphere.radius)
        {
            return false;
        }

    } // Next plane

    // Intersects
    return true;
}

bool frustum::test_sphere(const bsphere& sphere, const transform& t) const
{
    return test_sphere(bsphere(t.transform_coord(sphere.position), sphere.radius));
}

//-----------------------------------------------------------------------------
//  Name : sweptSphereIntersectPlane () (Private, Static)
/// <summary>
/// Determine whether or not the specified sphere, swept along the
/// provided direction vector, intersects a plane.
/// </summary>
//-----------------------------------------------------------------------------
bool frustum::swept_sphere_intersect_plane(float& t0,
                                           float& t1,
                                           const plane& plane,
                                           const bsphere& sphere,
                                           const vec3& vecSweepDirection)
{
    float b_dot_n = plane::dot_coord(plane, sphere.position);
    float d_dot_n = plane::dot_normal(plane, vecSweepDirection);

    if(d_dot_n == 0.0f)
    {
        if(b_dot_n <= sphere.radius)
        {
            //  Effectively infinity
            t0 = 0.0f;
            t1 = std::numeric_limits<float>::max();
            return true;

        } // End if infinity

        return false;

    } // End if runs parallel to plane

    // Compute the two possible intersections
    float tmp0 = (sphere.radius - b_dot_n) / d_dot_n;
    float tmp1 = (-sphere.radius - b_dot_n) / d_dot_n;
    t0 = glm::min<float>(tmp0, tmp1);
    t1 = glm::max<float>(tmp0, tmp1);
    return true;

    // End if intersection
}

//-----------------------------------------------------------------------------
//  Name : testSweptSphere ()
/// <summary>
/// Determine whether or not the specified sphere, swept along the
/// provided direction vector, intersects the frustum in some way.
/// </summary>
//-----------------------------------------------------------------------------
bool frustum::test_swept_sphere(const bsphere& sphere, const vec3& vecSweepDirection) const
{
    unsigned int i, nCount = 0;
    float t0, t1, fDisplacedRadius;
    float pDisplacements[12];
    vec3 vDisplacedCenter;

    // Determine all 12 intersection points of the swept sphere with the view
    // frustum.
    for(const auto& plane : planes)
    {
        // Intersects frustum plane?
        if(swept_sphere_intersect_plane(t0, t1, plane, sphere, vecSweepDirection))
        {
            // TODO: Possibly needs to be < 0?
            if(t0 >= 0.0f)
            {
                pDisplacements[nCount++] = t0;
            }
            if(t1 >= 0.0f)
            {
                pDisplacements[nCount++] = t1;
            }

        } // End if intersects

    } // Next plane

    // For all points > 0, displace the sphere along the sweep direction. If the
    // displaced
    // sphere falls inside the frustum then we have an intersection.
    for(i = 0; i < nCount; ++i)
    {
        vDisplacedCenter = sphere.position + (vecSweepDirection * pDisplacements[i]);
        fDisplacedRadius = sphere.radius * 1.1f; // Tolerance.
        if(test_sphere(bsphere(vDisplacedCenter, fDisplacedRadius)))
        {
            return true;
        }

    } // Next Intersection

    // None of the displaced spheres intersected the frustum
    return false;
}

//-----------------------------------------------------------------------------
//  Name : testPoint ()
/// <summary>
/// Determine whether or not the specified point falls within the frustum.
/// </summary>
//-----------------------------------------------------------------------------
bool frustum::test_point(const vec3& vecPoint) const
{
    return test_sphere(bsphere(vecPoint, 0.0f));
}

//-----------------------------------------------------------------------------
//  Name : testLine ()
/// <summary>
/// Determine whether or not the line passed is within the frustum.
/// </summary>
//-----------------------------------------------------------------------------
bool frustum::test_line(const vec3& v1, const vec3& v2) const
{
    int nCode1 = 0, nCode2 = 0;
    float fDist1, fDist2, t;
    int nSide1, nSide2;
    vec3 vDir, vIntersect;
    unsigned int i;

    // Test each plane
    for(i = 0; i < 6; ++i)
    {
        // Classify each point of the line against the plane.
        fDist1 = plane::dot_coord(planes[i], v1);
        fDist2 = plane::dot_coord(planes[i], v2);
        nSide1 = (fDist1 >= 0) ? 1 : 0;
        nSide2 = (fDist2 >= 0) ? 1 : 0;

        // Accumulate the classification info to determine
        // if the edge was spanning any of the planes.
        nCode1 |= (nSide1 << i);
        nCode2 |= (nSide2 << i);

        // If the line is completely in front of any plane
        // then it cannot possibly be intersecting.
        if(nSide1 == 1 && nSide2 == 1)
        {
            return false;
        }

        // The line is potentially spanning?
        if((nSide1 ^ nSide2) != 0)
        {
            // Compute the point at which the line intersects this plane.
            vDir = v2 - v1;
            t = -plane::dot_coord(planes[i], v1) / plane::dot_normal(planes[i], vDir);

            // Truly spanning?
            if((t >= 0.0f) && (t <= 1.0f))
            {
                vIntersect = v1 + (vDir * t);
                if(test_sphere(bsphere(vIntersect, 0.01f)))
                {
                    return true;
                }

            } // End if spanning

        } // End if different sides

    } // Next plane

    // Intersecting?
    return (nCode1 == 0) || (nCode2 == 0);
}

//-----------------------------------------------------------------------------
//  Name : classifyPlane ()
/// <summary>
/// Classify the frustum with respect to the plane
/// </summary>
//-----------------------------------------------------------------------------
volume_query frustum::classify_plane(const plane& plane) const
{
    unsigned int nInFrontCount = 0;
    unsigned int nBehindCount = 0;

    // Test frustum points
    for(unsigned int i = 0; i < 8; ++i)
    {
        float fDot = plane::dot_coord(plane, points[i]);
        if(fDot > 0.0f)
        {
            nInFrontCount++;
        }
        else if(fDot < 0.0f)
        {
            nBehindCount++;
        }

    } // Next plane

    // frustum entirely in front of plane
    if(nInFrontCount == 8)
    {
        return volume_query::outside;
    }

    // frustum entire behind plane
    if(nBehindCount == 8)
    {
        return volume_query::inside;
    }

    // Return intersection (spanning the plane)
    return volume_query::intersect;
}

//-----------------------------------------------------------------------------
//  Name : testFrustum ()
/// <summary>
/// Determine whether or not the frustum passed is within this one.
/// </summary>
//-----------------------------------------------------------------------------
bool frustum::test_frustum(const frustum& f) const
{
    // clang-format off
    // A -> B
    bool bIntersect1 =
        test_line(f.points[volume_geometry_point::left_bottom_far],
                  f.points[volume_geometry_point::left_bottom_near]) ||
        test_line(f.points[volume_geometry_point::left_bottom_near],
                  f.points[volume_geometry_point::right_bottom_near]) ||
        test_line(f.points[volume_geometry_point::right_bottom_near],
                  f.points[volume_geometry_point::right_bottom_far]) ||
        test_line(f.points[volume_geometry_point::right_bottom_far],
                  f.points[volume_geometry_point::left_bottom_far]) ||
        test_line(f.points[volume_geometry_point::right_bottom_far],
                  f.points[volume_geometry_point::right_top_far]) ||
        test_line(f.points[volume_geometry_point::right_bottom_near],
                  f.points[volume_geometry_point::right_top_near]) ||
        test_line(f.points[volume_geometry_point::left_bottom_far],
                  f.points[volume_geometry_point::left_top_far]) ||
        test_line(f.points[volume_geometry_point::left_bottom_near],
                  f.points[volume_geometry_point::left_top_near]) ||
        test_line(f.points[volume_geometry_point::left_top_near],
                  f.points[volume_geometry_point::left_top_far]) ||
        test_line(f.points[volume_geometry_point::left_top_far],
                  f.points[volume_geometry_point::right_top_far]) ||
        test_line(f.points[volume_geometry_point::right_top_far],
                  f.points[volume_geometry_point::right_top_near]) ||
        test_line(f.points[volume_geometry_point::right_top_near],
                  f.points[volume_geometry_point::left_top_near]);
    // clang-format on

    // Early out
    if(bIntersect1)
    {
        return true;
    }

    // clang-format off
    // B -> A
    bool bIntersect2 =
        f.test_line(points[volume_geometry_point::left_bottom_far],
                    points[volume_geometry_point::left_bottom_near]) ||
        f.test_line(points[volume_geometry_point::left_bottom_near],
                    points[volume_geometry_point::right_bottom_near]) ||
        f.test_line(points[volume_geometry_point::right_bottom_near],
                    points[volume_geometry_point::right_bottom_far]) ||
        f.test_line(points[volume_geometry_point::right_bottom_far],
                    points[volume_geometry_point::left_bottom_far]) ||
        f.test_line(points[volume_geometry_point::right_bottom_far],
                    points[volume_geometry_point::left_top_far]) ||
        f.test_line(points[volume_geometry_point::right_bottom_near],
                    points[volume_geometry_point::right_top_near]) ||
        f.test_line(points[volume_geometry_point::left_bottom_far],
                    points[volume_geometry_point::left_top_far]) ||
        f.test_line(points[volume_geometry_point::left_bottom_near],
                    points[volume_geometry_point::left_top_near]) ||
        f.test_line(points[volume_geometry_point::left_top_near],
                    points[volume_geometry_point::left_top_far]) ||
        f.test_line(points[volume_geometry_point::left_top_far],
                    points[volume_geometry_point::right_top_near]) ||
        f.test_line(points[volume_geometry_point::right_top_far],
                    points[volume_geometry_point::right_top_near]) ||
        f.test_line(points[volume_geometry_point::right_top_near],
                    points[volume_geometry_point::left_top_near]);
    // clang-format on

    // Return intersection result
    return bIntersect2;
}

//-----------------------------------------------------------------------------
//  Name : transform ()
/// <summary>
/// Transforms this frustum by the specified matrix.
/// </summary>
//-----------------------------------------------------------------------------
frustum& frustum::mul(const transform& mtx)
{
    auto mtxIT = transpose(inverse(mtx.get_matrix()));

    // transform planes
    for(auto& plane : planes)
    {
        plane = plane::normalize(plane::mul(plane, mtxIT));
    }

    // transform points
    for(auto& point : points)
    {
        point = mtx.transform_coord(point);
    }

    // transform originating position.
    position = mtx.transform_coord(position);

    // Return reference to self.
    return *this;
}

//-----------------------------------------------------------------------------
//  Name : operator== ( const frustum& )
/// <summary>
/// Determine whether or not the two frustums match.
/// </summary>
//-----------------------------------------------------------------------------
bool frustum::operator==(const frustum& frustum) const
{
    // Compare planes.
    for(size_t i = 0; i < planes.size(); ++i)
    {
        const plane& p1 = planes[i];
        const plane& p2 = frustum.planes[i];
        if(!(glm::abs<float>(p1.data.x - p2.data.x) <= glm::epsilon<float>() &&
             glm::abs<float>(p1.data.y - p2.data.y) <= glm::epsilon<float>() &&
             glm::abs<float>(p1.data.z - p2.data.z) <= glm::epsilon<float>() &&
             glm::abs<float>(p1.data.w - p2.data.w) <= glm::epsilon<float>()))
        {
            return false;
        }

    } // Next plane

    // Match
    return true;
}
} // namespace math
