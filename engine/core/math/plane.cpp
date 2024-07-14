#include "plane.h"

namespace math
{
auto plane::dot(const plane& p, const vec4& v) -> float
{
    return glm::dot(p.data, v);
}

auto plane::dot_coord(const plane& p, const vec3& v) -> float
{
    return glm::dot(vec3(p.data), v) + p.data.w;
}

auto plane::dot_normal(const plane& p, const vec3& v) -> float
{
    return glm::dot(vec3{p.data}, v);
}

auto plane::from_point_normal(const vec3& point, const vec3& normal) -> plane
{
    vec3 normalizedNormal = glm::normalize(normal);
    return plane(normalizedNormal.x, normalizedNormal.y, normalizedNormal.z, -glm::dot(point, normalizedNormal));
}

auto plane::from_points(const vec3& v1, const vec3& v2, const vec3& v3) -> plane
{
    vec3 normal = glm::normalize(glm::cross(v2 - v1, v3 - v1));
    return from_point_normal(v1, normal);
}

auto plane::mul(const plane& p, const mat4& m) -> plane
{
    return plane(m * p.data);
}

auto plane::normalize(const plane& p) -> plane
{
    float length = glm::length(vec3(p.data));
    return plane(p.data / length);
}

auto plane::scale(const plane& p, float s) -> plane
{
    return plane(p.data * s);
}

auto plane::operator*(float s) const -> plane
{
    return plane(data * s);
}

auto plane::operator/(float s) const -> plane
{
    return plane(data / s);
}

auto plane::operator*=(float s) -> plane&
{
    data *= s;
    return *this;
}

auto plane::operator/=(float s) -> plane&
{
    data /= s;
    return *this;
}

auto plane::operator+() const -> plane
{
    return *this;
}

auto plane::operator-() const -> plane
{
    return plane(-data);
}

auto plane::operator==(const plane& p) const -> bool
{
    return data == p.data;
}

auto plane::operator!=(const plane& p) const -> bool
{
    return data != p.data;
}

auto plane::operator=(const vec4& rhs) -> plane&
{
    data = rhs;
    return *this;
}

plane::plane() = default;

plane::plane(const vec4& p) : data(p)
{
}

plane::plane(float _a, float _b, float _c, float _d) : data(_a, _b, _c, _d)
{
}
} // namespace math
