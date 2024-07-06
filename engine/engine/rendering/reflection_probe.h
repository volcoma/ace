#pragma once
#include <math/math.h>
#include <reflection/registration.h>
#include <serialization/serialization.h>

#include <cstdint>

namespace ace
{
/**
 * @brief Enum class representing the type of reflection probe.
 */
enum class probe_type : std::uint8_t
{
    box = 0,   ///< Box type reflection probe.
    sphere = 1 ///< Sphere type reflection probe.
};

/**
 * @brief Enum class representing the reflection method.
 */
enum class reflect_method : std::uint8_t
{
    environment = 0, ///< Environment reflection method.
    static_only = 1  ///< Static-only reflection method.
};

/**
 * @brief Structure representing a reflection probe.
 */
struct reflection_probe
{
    REFLECTABLE(reflection_probe)
    SERIALIZABLE(reflection_probe)

    /**
     * @brief Structure representing box projection data.
     */
    struct box
    {
        math::vec3 extents = {5.0, 5.0f, 5.0f}; ///< Extents of the box projection.
        float transition_distance = 1.0f;       ///< Transition distance for the box projection.
    };

    /**
     * @brief Structure representing sphere projection data.
     */
    struct sphere
    {
        float range = 5.0f; ///< Range of the sphere projection.
    };

    probe_type type = probe_type::box;                   ///< Type of the reflection probe.
    reflect_method method = reflect_method::environment; ///< Reflection method.
    box box_data;                                        ///< Data describing box projection.
    sphere sphere_data;                                  ///< Data describing sphere projection.
};

/**
 * @brief Equality operator for reflection_probe.
 *
 * @param pr1 First reflection_probe to compare.
 * @param pr2 Second reflection_probe to compare.
 * @return true if the reflection probes are equal, false otherwise.
 */
inline bool operator==(const reflection_probe& pr1, const reflection_probe& pr2)
{
    return pr1.type == pr2.type && pr1.method == pr2.method && pr1.box_data.extents == pr2.box_data.extents &&
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
inline bool operator!=(const reflection_probe& pr1, const reflection_probe& pr2)
{
    return !(pr1 == pr2);
}

} // namespace ace
