#pragma once

//-----------------------------------------------------------------------------
// bbox Header Includes
//-----------------------------------------------------------------------------
#include "bsphere.h"
#include "math_types.h"
#include "plane.h"
#include "transform.hpp"
namespace math
{
using namespace glm;

/**
 * @class bbox
 * @brief Storage for box vector values and wraps up common functionality
 */
struct bbox
{
    /**
     * @brief Default Constructor
     */
    bbox();

    /**
     * @brief Constructor that sets values from vector values passed
     * @param minimum Minimum vector value
     * @param maximum Maximum vector value
     */
    bbox(const vec3& minimum, const vec3& maximum);

    /**
     * @brief Constructor that sets values from float values passed
     * @param xMin Minimum x-coordinate
     * @param yMin Minimum y-coordinate
     * @param zMin Minimum z-coordinate
     * @param xMax Maximum x-coordinate
     * @param yMax Maximum y-coordinate
     * @param zMax Maximum z-coordinate
     */
    bbox(float xMin, float yMin, float zMin, float xMax, float yMax, float zMax);

    /**
     * @brief Retrieves the plane for the specified side of the bounding box
     * @param side The side of the bounding box
     * @return The plane for the specified side
     */
    plane get_plane(volume_plane::e side) const;

    /**
     * @brief Retrieves the four points that form the boundary of the specified side of the bounding box
     * @param side The side of the bounding box
     * @param points_out Array to store the four points
     */
    void get_plane_points(volume_plane::e side, vec3 points_out[]) const;

    /**
     * @brief Calculates the bounding box based on the points specified
     * @param point_buffer Pointer to the buffer containing points
     * @param point_count Number of points
     * @param point_stride Stride between points
     * @param reset Reset the bounding box before calculation
     * @return Reference to the modified bounding box
     */
    bbox& from_points(const char* point_buffer, unsigned int point_count, unsigned int point_stride, bool reset = true);

    /**
     * @brief Calculates the bounding box based on the sphere specified
     * @param center Center of the sphere
     * @param radius Radius of the sphere
     * @return Reference to the modified bounding box
     */
    bbox& from_sphere(const vec3& center, float radius);

    /**
     * @brief Tests to see if this AABB is intersected by another AABB
     * @param bounds The other bounding box
     * @return True if intersected, false otherwise
     */
    bool intersect(const bbox& bounds) const;

    /**
     * @brief Tests to see if this AABB is intersected by another AABB with full containment test
     * @param bounds The other bounding box
     * @param contained Output parameter to indicate if fully contained
     * @return True if intersected, false otherwise
     */
    bool intersect(const bbox& bounds, bool& contained) const;

    /**
     * @brief Tests to see if this AABB is intersected by another AABB and returns the resulting intersection
     * @param bounds The other bounding box
     * @param intersection The resulting intersection bounding box
     * @return True if intersected, false otherwise
     */
    bool intersect(const bbox& bounds, bbox& intersection) const;

    /**
     * @brief Tests to see if this AABB is intersected by another AABB with a tolerance
     * @param bounds The other bounding box
     * @param tolerance The tolerance vector
     * @return True if intersected, false otherwise
     */
    bool intersect(const bbox& bounds, const vec3& tolerance) const;

    /**
     * @brief Tests to see if a ray intersects the AABB
     * @param origin The origin of the ray
     * @param velocity The direction and magnitude of the ray
     * @param t Output parameter for the intersection time
     * @param restrict_range Restrict the range of t to [0, 1]
     * @return True if intersected, false otherwise
     */
    bool intersect(const vec3& origin, const vec3& velocity, float& t, bool restrict_range = true) const;

    /**
     * @brief Tests to see if a triangle intersects the AABB
     * @param v0 First vertex of the triangle
     * @param v1 Second vertex of the triangle
     * @param v2 Third vertex of the triangle
     * @param triangle_bounds Bounding box of the triangle
     * @return True if intersected, false otherwise
     */
    bool intersect(const vec3& v0, const vec3& v1, const vec3& v2, const bbox& triangle_bounds) const;

    /**
     * @brief Tests to see if a triangle intersects the AABB
     * @param v0 First vertex of the triangle
     * @param v1 Second vertex of the triangle
     * @param v2 Third vertex of the triangle
     * @return True if intersected, false otherwise
     */
    bool intersect(const vec3& v0, const vec3& v1, const vec3& v2) const;

    /**
     * @brief Tests to see if a point falls within this bounding box or not
     * @param point The point to test
     * @return True if the point is within the bounding box, false otherwise
     */
    bool contains_point(const vec3& point) const;

    /**
     * @brief Tests to see if a point falls within this bounding box or not including a specific tolerance around the
     * box
     * @param point The point to test
     * @param tolerance The tolerance value
     * @return True if the point is within the bounding box with tolerance, false otherwise
     */
    bool contains_point(const vec3& point, const vec3& tolerance) const;

    /**
     * @brief Tests to see if a point falls within this bounding box or not including a specific tolerance around the
     * box
     * @param point The point to test
     * @param tolerance The tolerance value
     * @return True if the point is within the bounding box with tolerance, false otherwise
     */
    bool contains_point(const vec3& point, float tolerance) const;

    /**
     * @brief Computes the closest point on the surface of the AABB to the input point
     * @param source_point The input point
     * @return The closest point on the AABB
     */
    vec3 closest_point(const vec3& source_point) const;

    /**
     * @brief Ensures that the values placed in the min/max values never make the bounding box itself inverted
     */
    void validate();

    /**
     * @brief Resets the bounding box values
     */
    void reset();

    /**
     * @brief Transforms an axis aligned bounding box by the specified matrix
     * @param t The transformation matrix
     * @return Reference to the transformed bounding box
     */
    bbox& mul(const transform& t);

    /**
     * @brief Static method to transform the specified bounding box by the provided matrix and return the new resulting
     * box as a copy
     * @param bounds The bounding box to transform
     * @param t The transformation matrix
     * @return The transformed bounding box
     */
    static bbox mul(const bbox& bounds, const transform& t);

    /**
     * @brief Grows the bounding box based on the point passed
     * @param point The point to add
     * @return Reference to the modified bounding box
     */
    bbox& add_point(const vec3& point);

    /**
     * @brief Returns a vector containing the dimensions of the bounding box
     * @return The dimensions of the bounding box
     */
    vec3 get_dimensions() const;

    /**
     * @brief Returns a vector containing the exact center point of the box
     * @return The center point of the box
     */
    vec3 get_center() const;

    /**
     * @brief Returns a vector containing the extents of the bounding box (the half-dimensions)
     * @return The extents of the bounding box
     */
    vec3 get_extents() const;

    /**
     * @brief Grows the bounding box by the specified number of world space units on all three axes
     * @param amount The amount to grow the bounding box
     */
    void inflate(float amount);

    /**
     * @brief Grows the bounding box by the specified numbers of world space units on each of the three axes
     * independently
     * @param amount The amount to grow the bounding box on each axis
     */
    void inflate(const vec3& amount);

    /**
     * @brief Checks if the bounding box is populated
     * @return True if populated, false otherwise
     */
    bool is_populated() const;

    /**
     * @brief Checks if the bounding box is degenerate (empty)
     * @return True if degenerate, false otherwise
     */
    bool is_degenerate() const;

    /**
     * @brief Scales the bounding box values by the scalar passed
     * @param scale The scale factor
     * @return The scaled bounding box
     */
    bbox operator*(float scale) const;

    /**
     * @brief Moves the bounding box by the vector passed
     * @param shift The vector to move by
     * @return Reference to the modified bounding box
     */
    bbox& operator+=(const vec3& shift);

    /**
     * @brief Moves the bounding box by the vector passed
     * @param shift The vector to move by
     * @return Reference to the modified bounding box
     */
    bbox& operator-=(const vec3& shift);

    /**
     * @brief Transforms the bounding box by the matrix passed
     * @param t The transformation matrix
     * @return Reference to the modified bounding box
     */
    bbox& operator*=(const transform& t);

    /**
     * @brief Scales the bounding box values by the scalar passed
     * @param scale The scale factor
     * @return Reference to the modified bounding box
     */
    bbox& operator*=(float scale);

    /**
     * @brief Checks for equality between this bounding box and another
     * @param bounds The other bounding box
     * @return True if equal, false otherwise
     */
    bool operator==(const bbox& bounds) const;

    /**
     * @brief Checks for inequality between this bounding box and another
     * @param bounds The other bounding box
     * @return True if not equal, false otherwise
     */
    bool operator!=(const bbox& bounds) const;

    /**
     * @brief The minimum vector value of the bounding box
     */
    vec3 min;

    /**
     * @brief The maximum vector value of the bounding box
     */
    vec3 max;

    /**
     * @brief An empty bounding box
     */
    static bbox empty;
};
} // namespace math
