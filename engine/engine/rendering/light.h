#pragma once

#include <math/math.h>
#include <reflection/registration.h>
#include <serialization/serialization.h>

#include <cstdint>
namespace ace
{
enum class light_type : uint8_t
{
    spot = 0,
    point = 1,
    directional = 2,

    count
};

enum class sm_depth : uint8_t
{
    invz = 0,
    linear = 1,

    count
};

enum class pack_depth : uint8_t
{
    rgba = 0,
    vsm = 1,

    count
};

enum class sm_impl : uint8_t
{
    hard = 0,
    pcf = 1,
    vsm = 2,
    esm = 3,

    count
};

enum class sm_type : uint8_t
{
    single = 0,
    omni = 1,
    cascade = 2,

    count
};

enum class sm_resolution : uint8_t
{
    low,
    medium,
    high,
    very_high,

    count
};

struct light
{
    REFLECTABLE(light)
    SERIALIZABLE(light)

    light_type type = light_type::directional;

    struct spot
    {
        void set_range(float r);
        float get_range() const
        {
            return range;
        }

        void set_outer_angle(float angle);
        float get_outer_angle() const
        {
            return outer_angle;
        }

        void set_inner_angle(float angle);
        float get_inner_angle() const
        {
            return inner_angle;
        }

        float range = 10.0f;
        float outer_angle = 60.0f;
        float inner_angle = 30.0f;

        struct shadowmap_params
        {
        } shadow_params{};
    };

    struct point
    {
        float range = 10.0f;
        float exponent_falloff = 1.0f;

        struct shadowmap_params
        {
            float fov_x_adjust = 0.0f;
            float fov_y_adjust = 0.0f;
            bool stencil_pack = false;

        } shadow_params{};
    };

    struct directional
    {
        struct shadowmap_params
        {
            float split_distribution = 0.6f;
            uint8_t num_splits = 1;
            bool stabilize = true;
        } shadow_params{};
    };

    spot spot_data;
    point point_data;
    directional directional_data;
    math::color color = {1.0f, 1.0f, 1.0f, 1.0f};
    float intensity = 1.0f;

    struct shadowmap_params
    {
        sm_depth depth = sm_depth::invz;
        sm_impl type = sm_impl::pcf;

        sm_resolution resolution = sm_resolution::very_high;

        uint8_t size_power_of_two{10};
        float depth_value_pow{1.0f};
        float near_plane{0.2f};
        float far_plane{550.0f};
        float bias{0.0012f};
        float normal_bias{0.001f};
        float custom_param0{};
        float custom_param1{};
        float x_num{2};
        float y_num{2};
        float x_offset{1};
        float y_offset{1};
        bool do_blur{true};

        bool show_coverage{false};

    } shadow_params;
};
} // namespace ace
