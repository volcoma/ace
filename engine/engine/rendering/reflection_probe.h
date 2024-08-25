#pragma once
#include <math/math.h>

#include <cstdint>

namespace ace
{
/**
 * @brief Enum class representing the type of reflection probe.
 */
enum class probe_type : std::uint8_t
{
    /// Box type reflection probe.
    box = 0,
    /// Sphere type reflection probe.
    sphere = 1
};

/**
 * @brief Enum class representing the reflection method.
 */
enum class reflect_method : std::uint8_t
{
    /// Environment reflection method.
    environment = 0,
    /// Static-only reflection method.
    static_only = 1
};

/**
 * @brief Structure representing a reflection probe.
 */
struct reflection_probe
{
    /**
     * @brief Structure representing box projection data.
     */
    struct box
    {
        /// Extents of the box projection.
        math::vec3 extents = {5.0, 5.0f, 5.0f};
        /// Transition distance for the box projection.
        float transition_distance = 1.0f;
    };

    /**
     * @brief Structure representing sphere projection data.
     */
    struct sphere
    {
        /// Range of the sphere projection.
        float range = 5.0f;
    };

    auto get_face_extents(uint32_t face, const math::transform& transform) const -> float
    {
        const auto& scale = transform.get_scale();

        if(type == probe_type::sphere)
        {
            auto max_scale = math::max(scale.x, math::max(scale.y, scale.z));
            return sphere_data.range * max_scale;
        }

        if(type == probe_type::box)
        {
            auto scaled_extents = box_data.extents * scale;
            return math::max(scaled_extents.x, math::max(scaled_extents.y, scaled_extents.z));
        }

        return {};
    }

    auto get_max_range() const -> float
    {
        if(type == probe_type::sphere)
        {
            return sphere_data.range;
        }
        else if(type == probe_type::box)
        {
            return math::max(box_data.extents.x, math::max(box_data.extents.y, box_data.extents.z));
        }
        return 0.0f;
    }

    /// Type of the reflection probe.
    probe_type type = probe_type::box;
    /// Reflection method.
    reflect_method method = reflect_method::environment;
    /// Intensity
    float intensity = 1.0f;
    /// Data describing box projection.
    box box_data;
    /// Data describing sphere projection.
    sphere sphere_data;
};

/**
 * @brief Equality operator for reflection_probe.
 *
 * @param pr1 First reflection_probe to compare.
 * @param pr2 Second reflection_probe to compare.
 * @return true if the reflection probes are equal, false otherwise.
 */
inline auto operator==(const reflection_probe& pr1, const reflection_probe& pr2) -> bool
{
    return pr1.type == pr2.type && pr1.method == pr2.method &&
           math::equal(pr1.intensity, pr2.intensity, math::epsilon<float>()) &&
           math::all(math::equal(pr1.box_data.extents, pr2.box_data.extents, math::epsilon<float>())) &&
           math::equal(pr1.box_data.transition_distance, pr2.box_data.transition_distance, math::epsilon<float>()) &&
           math::equal(pr1.sphere_data.range, pr2.sphere_data.range, math::epsilon<float>());
}

/**
 * @brief Inequality operator for reflection_probe.
 *
 * @param pr1 First reflection_probe to compare.
 * @param pr2 Second reflection_probe to compare.
 * @return true if the reflection probes are not equal, false otherwise.
 */
inline auto operator!=(const reflection_probe& pr1, const reflection_probe& pr2) -> bool
{
    return !(pr1 == pr2);
}

} // namespace ace
