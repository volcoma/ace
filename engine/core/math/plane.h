#pragma once

#include "transform.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace math
{
using namespace glm;

//-----------------------------------------------------------------------------
// Main class declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : plane (Class)
/// <summary>
/// Storage for infinite plane.
/// </summary>
//-----------------------------------------------------------------------------
struct plane
{
    //-------------------------------------------------------------------------
    // Friend List
    //-------------------------------------------------------------------------

    /**
     * @brief Scalar multiplication for a plane.
     *
     * Multiplies each component of the plane by a scalar value.
     *
     * @param s Scalar value to multiply by.
     * @param p The plane to be multiplied.
     * @return A new plane with each component multiplied by the scalar value.
     */
    friend auto operator*(float s, const plane& p) -> plane;

    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------

    /**
     * @brief Default constructor.
     *
     * Initializes the plane with all components set to zero.
     */
    plane();

    /**
     * @brief Constructor from a vec4.
     *
     * Initializes the plane with the given vec4 components.
     *
     * @param p A vec4 representing the plane components.
     */
    plane(const vec4& p);

    /**
     * @brief Constructor from individual float components.
     *
     * Initializes the plane with the given components.
     *
     * @param _a The x component of the plane normal.
     * @param _b The y component of the plane normal.
     * @param _c The z component of the plane normal.
     * @param _d The distance component of the plane.
     */
    plane(float _a, float _b, float _c, float _d);

    //-------------------------------------------------------------------------
    // Public Static Methods
    //-------------------------------------------------------------------------

    /**
     * @brief Computes the dot product of the plane and a vec4.
     *
     * @param p The plane.
     * @param v The vec4.
     * @return The dot product as a float.
     */
    static auto dot(const plane& p, const vec4& v) -> float;

    /**
     * @brief Computes the dot product of the plane and a vec3 (considering the plane's distance).
     *
     * @param p The plane.
     * @param v The vec3.
     * @return The dot product as a float.
     */
    static auto dot_coord(const plane& p, const vec3& v) -> float;

    /**
     * @brief Computes the dot product of the plane normal and a vec3.
     *
     * @param p The plane.
     * @param v The vec3.
     * @return The dot product as a float.
     */
    static auto dot_normal(const plane& p, const vec3& v) -> float;

    /**
     * @brief Creates a plane from a point and a normal.
     *
     * @param point A point on the plane.
     * @param normal The normal vector of the plane.
     * @return A new plane defined by the point and normal.
     */
    static auto from_point_normal(const vec3& point, const vec3& normal) -> plane;

    /**
     * @brief Creates a plane from three points.
     *
     * @param v1 The first point.
     * @param v2 The second point.
     * @param v3 The third point.
     * @return A new plane defined by the three points.
     */
    static auto from_points(const vec3& v1, const vec3& v2, const vec3& v3) -> plane;

    /**
     * @brief Transforms a plane by a 4x4 matrix.
     *
     * @param p The plane to be transformed.
     * @param m The transformation matrix.
     * @return A new transformed plane.
     */
    static auto mul(const plane& p, const mat4& m) -> plane;

    /**
     * @brief Normalizes the plane.
     *
     * Normalizes the plane's normal vector and adjusts the distance component.
     *
     * @param p The plane to be normalized.
     * @return A new normalized plane.
     */
    static auto normalize(const plane& p) -> plane;

    /**
     * @brief Scales the plane by a scalar value.
     *
     * Multiplies each component of the plane by the given scalar value.
     *
     * @param p The plane to be scaled.
     * @param s The scalar value to multiply by.
     * @return A new scaled plane.
     */
    static auto scale(const plane& p, float s) -> plane;

    //-------------------------------------------------------------------------
    // Public Operators
    //-------------------------------------------------------------------------
    /**
     * @brief Multiplies the plane by a scalar value.
     *
     * @param s The scalar value to multiply by.
     * @return A new plane with each component multiplied by the scalar value.
     */
    auto operator*(float s) const -> plane;

    /**
     * @brief Divides the plane by a scalar value.
     *
     * @param s The scalar value to divide by.
     * @return A new plane with each component divided by the scalar value.
     */
    auto operator/(float s) const -> plane;

    /**
     * @brief Multiplies and assigns the plane by a scalar value.
     *
     * @param s The scalar value to multiply by.
     * @return A reference to the plane after multiplication.
     */
    auto operator*=(float s) -> plane&;

    /**
     * @brief Divides and assigns the plane by a scalar value.
     *
     * @param s The scalar value to divide by.
     * @return A reference to the plane after division.
     */
    auto operator/=(float s) -> plane&;

    /**
     * @brief Unary plus operator.
     *
     * @return A copy of the plane.
     */
    auto operator+() const -> plane;

    /**
     * @brief Unary minus operator.
     *
     * Negates all components of the plane.
     *
     * @return A new plane with all components negated.
     */
    auto operator-() const -> plane;

    /**
     * @brief Equality operator.
     *
     * Checks if two planes are equal.
     *
     * @param p The plane to compare with.
     * @return True if the planes are equal, false otherwise.
     */
    auto operator==(const plane& p) const -> bool;

    /**
     * @brief Inequality operator.
     *
     * Checks if two planes are not equal.
     *
     * @param p The plane to compare with.
     * @return True if the planes are not equal, false otherwise.
     */
    auto operator!=(const plane& p) const -> bool;

    /**
     * @brief Assignment operator from vec4.
     *
     * Assigns the components of the plane from a vec4.
     *
     * @param rhs The vec4 to assign from.
     * @return A reference to the plane after assignment.
     */
    auto operator=(const vec4& rhs) -> plane&;

    //-------------------------------------------------------------------------
    // Public Members
    //-------------------------------------------------------------------------

    /**
     * @brief The components of the plane.
     *
     * The first three components represent the normal vector of the plane,
     * and the fourth component represents the distance from the origin.
     */
    vec4 data = {0.0f, 0.0f, 0.0f, 0.0f};
};

//-----------------------------------------------------------------------------
// Global Inline Operators (plane)
//-----------------------------------------------------------------------------
/**
 * @brief Scalar multiplication for a plane.
 *
 * Multiplies each component of the plane by a scalar value.
 *
 * @param s Scalar value to multiply by.
 * @param p The plane to be multiplied.
 * @return A new plane with each component multiplied by the scalar value.
 */
inline auto operator*(float s, const plane& p) -> plane
{
    return plane(p.data * s);
}
} // namespace math
