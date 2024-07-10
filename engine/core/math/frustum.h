#pragma once

#include "bsphere.h"
#include "bbox.h"
#include "bbox_extruded.h"
#include "math_types.h"
#include "plane.h"
#include "transform.hpp"
#include <array>

namespace math
{
using namespace glm;

//-----------------------------------------------------------------------------
// Main class declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : frustum (Class)
/// <summary>
/// Storage for frustum planes / values and wraps up common functionality
/// </summary>
//-----------------------------------------------------------------------------
class frustum
{
public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
    frustum();
    frustum(const transform& view, const transform& proj, bool _oglNDC);
    frustum(const bbox& sourceBounds);

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------
    void update(const transform& view, const transform& proj, bool _oglNDC);
    void set_planes(const std::array<plane, 6>& new_planes);
    void recompute_points();
    volume_query classify_vertices(const vec3* vertices, size_t count) const;
    volume_query classify_aabb(const bbox& bounds) const;
    volume_query classify_aabb(const bbox& bounds, unsigned int& frustumBits, int& lastOutside) const;
    volume_query classify_obb(const bbox& bounds, const transform& t) const;

    volume_query classify_sphere(const bsphere& sphere) const;
    volume_query classify_plane(const plane& plane) const;
    bool test_vertices(const vec3* vertices, size_t count) const;
    bool test_point(const vec3& point) const;
    bool test_aabb(const bbox& bounds) const;
    bool test_obb(const bbox& bounds, const transform& t) const;

    bool test_extruded_aabb(const bbox_extruded& box) const;
    bool test_extruded_obb(const bbox_extruded& box, const transform& t) const;
    bool test_sphere(const bsphere& sphere) const;
    bool test_sphere(const bsphere& sphere, const transform& t) const;

    bool test_swept_sphere(const bsphere& sphere, const vec3& sweepDirection) const;
    bool test_frustum(const frustum& frustum) const;
    bool test_line(const vec3& v1, const vec3& v2) const;
    frustum& mul(const transform& t);

    //-------------------------------------------------------------------------
    // Public Operators
    //-------------------------------------------------------------------------
    bool operator==(const frustum& f) const;

    //-------------------------------------------------------------------------
    // Public Variables
    //-------------------------------------------------------------------------
    std::array<plane, 6> planes;        // The 6 planes of the frustum.
    std::array<vec3, 8> points;         // The 8 corner points of the frustum.
    vec3 position = {0.0f, 0.0f, 0.0f}; // The originating position of the frustum.

private:
    //-------------------------------------------------------------------------
    // Private Static Functions
    //-------------------------------------------------------------------------
    static bool swept_sphere_intersect_plane(float& t0,
                                             float& t1,
                                             const plane& plane,
                                             const bsphere& sphere,
                                             const vec3& sweepDirection);
};
} // namespace math
