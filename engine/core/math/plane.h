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
    friend auto operator*(float, const plane&) -> plane;

    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------
    plane();
    plane(const vec4& p);
    plane(float _a, float _b, float _c, float _d);

    //-------------------------------------------------------------------------
    // Public Static Methods
    //-------------------------------------------------------------------------
    static auto dot(const plane& p, const vec4& v) -> float;
    static auto dot_coord(const plane& p, const vec3& v) -> float;
    static auto dot_normal(const plane& p, const vec3& v) -> float;
    static auto from_point_normal(const vec3& point, const vec3& normal) -> plane;
    static auto from_points(const vec3& v1, const vec3& v2, const vec3& v3) -> plane;
    static auto mul(const plane& p, const mat4& m) -> plane;
    static auto normalize(const plane& p) -> plane;
    static auto scale(const plane& p, float s) -> plane;

    //-------------------------------------------------------------------------
    // Public Operators
    //-------------------------------------------------------------------------
    auto operator*(float s) const -> plane;
    auto operator/(float s) const -> plane;
    auto operator*=(float s) -> plane&;
    auto operator/=(float s) -> plane&;
    auto operator+() const -> plane;
    auto operator-() const -> plane;
    auto operator==(const plane& p) const -> bool;
    auto operator!=(const plane& p) const -> bool;
    auto operator=(const vec4& rhs) -> plane&;

    //-------------------------------------------------------------------------
    // Public Members
    //-------------------------------------------------------------------------
    vec4 data = {0.0f, 0.0f, 0.0f, 0.0f};
};

//-----------------------------------------------------------------------------
// Global Inline Operators (plane)
//-----------------------------------------------------------------------------
inline auto operator*(float s, const plane& p) -> plane
{
    return plane(p.data * s);
}
} // namespace math
