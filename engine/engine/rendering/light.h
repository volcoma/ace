#pragma once

#include <engine/engine_export.h>

#include <cstdint>
#include <math/math.h>

namespace ace
{
/**
 * @brief Enum representing the type of light.
 */
enum class light_type : uint8_t
{
    spot = 0,
    point = 1,
    directional = 2,

    count
};

/**
 * @brief Enum representing the depth method for shadow mapping.
 */
enum class sm_depth : uint8_t
{
    invz = 0,
    linear = 1,

    count
};

/**
 * @brief Enum representing the packing method for depth in shadow mapping.
 */
enum class pack_depth : uint8_t
{
    rgba = 0,
    vsm = 1,

    count
};

/**
 * @brief Enum representing the implementation type for shadow mapping.
 */
enum class sm_impl : uint8_t
{
    hard = 0,
    pcf = 1,
    pcss = 2,
    vsm = 3,
    esm = 4,

    count,
};

/**
 * @brief Enum representing the type of shadow map.
 */
enum class sm_type : uint8_t
{
    single = 0,
    omni = 1,
    cascade = 2,

    count
};

/**
 * @brief Enum representing the resolution of shadow maps.
 */
enum class sm_resolution : uint8_t
{
    low,
    medium,
    high,
    very_high,

    count
};

/**
 * @brief Struct representing a light.
 */
struct light
{
    /// The type of the light.
    light_type type = light_type::directional;

    /**
     * @brief Struct representing spot light specific properties.
     */
    struct spot
    {
        /**
         * @brief Sets the range of the spot light.
         * @param r The range to set.
         */
        void set_range(float r);

        /**
         * @brief Gets the range of the spot light.
         * @return The range of the spot light.
         */
        float get_range() const
        {
            return range;
        }

        /**
         * @brief Sets the outer angle of the spot light.
         * @param angle The outer angle to set.
         */
        void set_outer_angle(float angle);

        /**
         * @brief Gets the outer angle of the spot light.
         * @return The outer angle of the spot light.
         */
        float get_outer_angle() const
        {
            return outer_angle;
        }

        /**
         * @brief Sets the inner angle of the spot light.
         * @param angle The inner angle to set.
         */
        void set_inner_angle(float angle);

        /**
         * @brief Gets the inner angle of the spot light.
         * @return The inner angle of the spot light.
         */
        float get_inner_angle() const
        {
            return inner_angle;
        }

        /// The range of the spot light.
        float range = 10.0f;
        /// The outer angle of the spot light.
        float outer_angle = 60.0f;
        /// The inner angle of the spot light.
        float inner_angle = 30.0f;

        /**
         * @brief Struct representing shadow map parameters for spot lights.
         */
        struct shadowmap_params
        {
        } shadow_params{};
    };

    /**
     * @brief Struct representing point light specific properties.
     */
    struct point
    {
        /// The range of the point light.
        float range = 10.0f;
        /// The exponent falloff for the point light.
        float exponent_falloff = 1.0f;

        /**
         * @brief Struct representing shadow map parameters for point lights.
         */
        struct shadowmap_params
        {
            /// Field of view x-axis adjustment.
            float fov_x_adjust = 0.0f;
            /// Field of view y-axis adjustment.
            float fov_y_adjust = 0.0f;
            /// Whether to use stencil packing.
            bool stencil_pack = false;

        } shadow_params{};
    };

    /**
     * @brief Struct representing directional light specific properties.
     */
    struct directional
    {
        /**
         * @brief Struct representing shadow map parameters for directional lights.
         */
        struct shadowmap_params
        {
            /// Split distribution for cascade shadow maps.
            float split_distribution = 0.8f;
            /// Number of splits for cascade shadow maps.
            uint8_t num_splits = 4;
            /// Whether to stabilize the shadow map.
            bool stabilize = true;
        } shadow_params{};
    };

    /// Data specific to spot lights.
    spot spot_data;
    /// Data specific to point lights.
    point point_data;
    /// Data specific to directional lights.
    directional directional_data;
    /// The color of the light.
    math::color color = {1.0f, 1.0f, 1.0f, 1.0f};
    /// The intensity of the light.
    float intensity = 1.0f;

    /// Whether the light casts shadows.
    bool casts_shadows{true};

    /**
     * @brief Struct representing common shadow map parameters.
     */
    struct shadowmap_params
    {
        /// Depth method for shadow mapping.
        sm_depth depth = sm_depth::invz;
        /// Implementation type for shadow mapping.
        sm_impl type = sm_impl::pcf;
        /// Resolution of the shadow map.
        sm_resolution resolution = sm_resolution::very_high;
        /// Size of the shadow map as a power of two.
        uint8_t size_power_of_two{10};
        /// Near plane distance for shadow mapping.
        float near_plane{0.2f};
        /// Far plane distance for shadow mapping.
        float far_plane{550.0f};
        /// Bias for shadow mapping.
        float bias{0.00115f};
        /// Normal bias for shadow mapping.
        float normal_bias{0.015f};


        // struct impl_params
        // {
        //     /// Custom parameter 0 for shadow mapping.
        //     float hardness{};
        //     /// Custom parameter 1 for shadow mapping.
        //     float depth_multiplier{};
        //     /// Number of shadow maps along the x-axis.
        //     float blur_x_num{2};
        //     /// Number of shadow maps along the y-axis.
        //     float blur_y_num{2};
        //     /// Offset along the x-axis for shadow mapping.
        //     float blur_x_offset{1};
        //     /// Offset along the y-axis for shadow mapping.
        //     float blur_y_offset{1};
        //     /// Whether to perform blur on the shadow map.
        //     bool do_blur{true};
        // } impl;

        /// Whether to show shadow map coverage.
        bool show_coverage{false};

    } shadow_params;
};

} // namespace ace
