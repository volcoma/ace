/*
 * This example demonstrates:
 * - Usage of Perez sky model [1] to render a dynamic sky.
 * - Rendering a mesh with a lightmap, shading of which is driven by the same parameters as the sky.
 *
 * Typically, the sky is rendered using cubemaps or other environment maps.
 * This approach can provide a high-quality sky, but the downside is that the
 * image is static. To achieve daytime changes in sky appearance, there is a need
 * in a dynamic model.
 *
 * Perez "An All-Weather Model for Sky Luminance Distribution" is a simple,
 * but good enough model which is, in essence, a function that
 * interpolates a sky color. As input, it requires several turbidity
 * coefficients, a color at zenith and direction to the sun.
 * Turbidity coefficients are taken from [2], which are computed using more
 * complex physically based models. Color at zenith depends on daytime and can
 * vary depending on many factors.
 *
 * In the code below, there are two tables that contain sky and sun luminance
 * which were computed using code from [3]. Luminance in those tables
 * represents actual scale of light energy that comes from sun compared to
 * the sky.
 *
 * The sky is driven by luminance of the sky, while the material of the
 * landscape is driven by both, the luminance of the sky and the sun. The
 * lightening model is very simple and consists of two parts: directional
 * light and hemisphere light. The first is used for the sun while the second
 * is used for the sky. Additionally, the second part is modulated by a
 * lightmap to achieve ambient occlusion effect.
 *
 * References
 * ==========
 *
 * [1] R. Perez, R. Seals, and J. Michalsky."An All-Weather Model for Sky Luminance Distribution".
 *     Solar Energy, Volume 50, Number 3 (March 1993), pp. 235-245.
 *
 * [2] A. J. Preetham, Peter Shirley, and Brian Smits. "A Practical Analytic Model for Daylight",
 *     Proceedings of the 26th Annual Conference on Computer Graphics and Interactive Techniques,
 *     1999, pp. 91-100.
 *     https://www.cs.utah.edu/~shirley/papers/sunsky/sunsky.pdf
 *
 * [3] E. Lengyel, Game Engine Gems, Volume One. Jones & Bartlett Learning, 2010. pp. 219 - 234
 *
 */

#pragma once

#include <engine/rendering/camera.h>
#include <engine/rendering/gpu_program.h>

#include <graphics/index_buffer.h>
#include <graphics/vertex_buffer.h>
#include <graphics/vertex_decl.h>

namespace ace
{

namespace detail
{

// Controls sun position according to time, month, and observer's latitude.
// Sun position computation based on Earth's orbital elements:
// https://nssdc.gsfc.nasa.gov/planetary/factsheet/earthfact.html
class sun_controller
{
public:
    enum class month : int
    {
        January = 0,
        February,
        March,
        April,
        May,
        June,
        July,
        August,
        September,
        October,
        November,
        December
    };

    sun_controller()
        : north_dir_(1.0f, 0.0f, 0.0f)
        , sun_dir_(0.0f, -1.0f, 0.0f)
        , up_dir_(0.0f, 1.0f, 0.0f)
        , latitude_(50.0f)
        , month_(month::June)
        , ecliptic_obliquity_(bx::toRad(23.4f))
        , delta_(0.0f)
    {
    }

    void update(float _time)
    {
        calculate_sun_orbit();
        update_sun_position(_time - 12.0f);
    }

    bx::Vec3 north_dir_;
    bx::Vec3 sun_dir_;
    bx::Vec3 up_dir_;
    float latitude_;
    month month_;

private:
    void calculate_sun_orbit()
    {
        const float day = 30.0f * float(month_) + 15.0f;
        float lambda = 280.46f + 0.9856474f * day;
        lambda = bx::toRad(lambda);
        delta_ = bx::asin(bx::sin(ecliptic_obliquity_) * bx::sin(lambda));
    }

    void update_sun_position(float _hour)
    {
        const float latitude = bx::toRad(latitude_);
        const float hh = _hour * bx::kPi / 12.0f;
        const float azimuth =
            bx::atan2(bx::sin(hh), bx::cos(hh) * bx::sin(latitude) - bx::tan(delta_) * bx::cos(latitude));

        const float altitude =
            bx::asin(bx::sin(latitude) * bx::sin(delta_) + bx::cos(latitude) * bx::cos(delta_) * bx::cos(hh));

        const bx::Quaternion rot0 = bx::fromAxisAngle(up_dir_, -azimuth);
        const bx::Vec3 dir = bx::mul(north_dir_, rot0);
        const bx::Vec3 uxd = bx::cross(up_dir_, dir);

        const bx::Quaternion rot1 = bx::fromAxisAngle(uxd, altitude);
        sun_dir_ = bx::mul(dir, rot1);
    }

    float ecliptic_obliquity_;
    float delta_;
};

} // namespace detail

class atmospheric_pass_perez
{
public:
    struct run_params
    {
        math::vec3 light_direction = math::normalize(math::vec3(0.2f, -0.8f, 1.0f));
    };

    auto init(rtti::context& ctx) -> bool;
    auto run(gfx::frame_buffer::ptr input, const camera& camera, delta_t dt, const run_params& params)
        -> gfx::frame_buffer::ptr;

private:
    /// Program that is responsible for rendering.
    std::unique_ptr<gpu_program> program_;

    std::unique_ptr<gfx::vertex_buffer> vb_;
    std::unique_ptr<gfx::index_buffer> ib_;

    detail::sun_controller sun_;

    float hour_{};
    float time_scale_{1.0f};

    // [1.9 - 10.0f]
    float turbidity_ = 1.9f;
};
} // namespace ace
