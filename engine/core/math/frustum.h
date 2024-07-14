#pragma once

#include "bbox.h"
#include "bsphere.h"
#include "math_types.h"
#include "plane.h"
#include "transform.hpp"
#include <array>

namespace math
{
using namespace glm;

/**
 * @brief Storage for frustum planes / values and wraps up common functionality.
 */
class frustum
{
public:
    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------

    /**
     * @brief Default constructor.
     */
    frustum();

    /**
     * @brief Constructs a frustum from view and projection matrices.
     *
     * @param view The view transform.
     * @param proj The projection transform.
     * @param _oglNDC Whether the frustum uses OpenGL's NDC.
     */
    frustum(const transform& view, const transform& proj, bool _oglNDC);

    /**
     * @brief Constructs a frustum from an axis-aligned bounding box.
     *
     * @param sourceBounds The bounding box to construct the frustum from.
     */
    frustum(const bbox& sourceBounds);

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------

    /**
     * @brief Updates the frustum based on the specified view and projection matrices.
     *
     * @param view The view transform.
     * @param proj The projection transform.
     * @param _oglNDC Whether the frustum uses OpenGL's NDC.
     */
    void update(const transform& view, const transform& proj, bool _oglNDC);

    /**
     * @brief Sets the frustum planes.
     *
     * @param new_planes The new planes to set.
     */
    void set_planes(const std::array<plane, 6>& new_planes);

    /**
     * @brief Recomputes the frustum's corner points based on its planes.
     */
    void recompute_points();

    /**
     * @brief Classifies vertices with respect to the frustum.
     *
     * @param vertices The vertices to classify.
     * @param count The number of vertices.
     * @return The classification result.
     */
    auto classify_vertices(const vec3* vertices, size_t count) const -> volume_query;

    /**
     * @brief Classifies an axis-aligned bounding box (AABB) with respect to the frustum.
     *
     * @param bounds The bounding box to classify.
     * @return The classification result.
     */
    auto classify_aabb(const bbox& bounds) const -> volume_query;

    /**
     * @brief Classifies an axis-aligned bounding box (AABB) with respect to the frustum.
     *
     * @param bounds The bounding box to classify.
     * @param frustumBits Bits representing the frustum planes.
     * @param lastOutside Index of the last outside plane.
     * @return The classification result.
     */
    auto classify_aabb(const bbox& bounds, unsigned int& frustumBits, int& lastOutside) const -> volume_query;

    /**
     * @brief Classifies an oriented bounding box (OBB) with respect to the frustum.
     *
     * @param bounds The bounding box to classify.
     * @param t The transform to apply to the bounding box.
     * @return The classification result.
     */
    auto classify_obb(const bbox& bounds, const transform& t) const -> volume_query;

    /**
     * @brief Classifies a sphere with respect to the frustum.
     *
     * @param sphere The sphere to classify.
     * @return The classification result.
     */
    auto classify_sphere(const bsphere& sphere) const -> volume_query;

    /**
     * @brief Classifies a plane with respect to the frustum.
     *
     * @param plane The plane to classify.
     * @return The classification result.
     */
    auto classify_plane(const plane& plane) const -> volume_query;

    /**
     * @brief Tests if vertices are inside or intersect the frustum.
     *
     * @param vertices The vertices to test.
     * @param count The number of vertices.
     * @return True if any vertex is inside or intersecting the frustum, false otherwise.
     */
    auto test_vertices(const vec3* vertices, size_t count) const -> bool;

    /**
     * @brief Tests if a point is inside or intersecting the frustum.
     *
     * @param point The point to test.
     * @return True if the point is inside or intersecting the frustum, false otherwise.
     */
    auto test_point(const vec3& point) const -> bool;

    /**
     * @brief Tests if an axis-aligned bounding box (AABB) is inside or intersecting the frustum.
     *
     * @param bounds The bounding box to test.
     * @return True if the bounding box is inside or intersecting the frustum, false otherwise.
     */
    auto test_aabb(const bbox& bounds) const -> bool;

    /**
     * @brief Tests if an oriented bounding box (OBB) is inside or intersecting the frustum.
     *
     * @param bounds The bounding box to test.
     * @param t The transform to apply to the bounding box.
     * @return True if the bounding box is inside or intersecting the frustum, false otherwise.
     */
    auto test_obb(const bbox& bounds, const transform& t) const -> bool;

    /**
     * @brief Tests if a sphere is inside or intersecting the frustum.
     *
     * @param sphere The sphere to test.
     * @return True if the sphere is inside or intersecting the frustum, false otherwise.
     */
    auto test_sphere(const bsphere& sphere) const -> bool;

    /**
     * @brief Tests if a transformed sphere is inside or intersecting the frustum.
     *
     * @param sphere The sphere to test.
     * @param t The transform to apply to the sphere.
     * @return True if the transformed sphere is inside or intersecting the frustum, false otherwise.
     */
    auto test_sphere(const bsphere& sphere, const transform& t) const -> bool;

    /**
     * @brief Determine whether or not the specified sphere, swept along the provided direction vector, intersects the
     * frustum in some way.
     *
     * @param sphere The sphere to test.
     * @param sweepDirection The direction to sweep the sphere.
     * @return True if the swept sphere intersects the frustum, false otherwise.
     */
    auto test_swept_sphere(const bsphere& sphere, const vec3& sweepDirection) const -> bool;

    /**
     * @brief Tests if another frustum intersects this frustum.
     *
     * @param frustum The other frustum to test.
     * @return True if the frustums intersect, false otherwise.
     */
    auto test_frustum(const frustum& frustum) const -> bool;

    /**
     * @brief Tests if a line intersects the frustum.
     *
     * @param v1 The start point of the line.
     * @param v2 The end point of the line.
     * @return True if the line intersects the frustum, false otherwise.
     */
    auto test_line(const vec3& v1, const vec3& v2) const -> bool;

    /**
     * @brief Multiplies the frustum by a transform.
     *
     * @param t The transform to multiply by.
     * @return A reference to the multiplied frustum.
     */
    auto mul(const transform& t) -> frustum&;

    //-------------------------------------------------------------------------
    // Public Operators
    //-------------------------------------------------------------------------

    /**
     * @brief Equality operator.
     *
     * @param f The frustum to compare with.
     * @return True if the frustums are equal, false otherwise.
     */
    auto operator==(const frustum& f) const -> bool;

    //-------------------------------------------------------------------------
    // Public Variables
    //-------------------------------------------------------------------------

    ///< The 6 planes of the frustum.
    std::array<plane, 6> planes;
    ///< The 8 corner points of the frustum.
    std::array<vec3, 8> points;
    ///< The originating position of the frustum.
    vec3 position = {0.0f, 0.0f, 0.0f};

private:
    //-------------------------------------------------------------------------
    // Private Static Functions
    //-------------------------------------------------------------------------

    /**
     * @brief Determine whether or not the specified sphere, swept along the provided direction vector, intersects a
     * plane.
     *
     * @param t0 The first intersection time.
     * @param t1 The second intersection time.
     * @param plane The plane to test against.
     * @param sphere The sphere to test.
     * @param sweepDirection The direction to sweep the sphere.
     * @return True if the swept sphere intersects the plane, false otherwise.
     */
    static auto swept_sphere_intersect_plane(float& t0,
                                             float& t1,
                                             const plane& plane,
                                             const bsphere& sphere,
                                             const vec3& sweepDirection) -> bool;
};
} // namespace math
