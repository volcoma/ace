#include "atmospheric_pass_perez.h"
#include <engine/assets/asset_manager.h>
#include <graphics/render_pass.h>
#include <graphics/texture.h>

namespace ace
{

namespace
{

// Represents color. Color-space depends on context.
// In the code below, used to represent color in XYZ, and RGB color-space
typedef bx::Vec3 Color;

// Performs piecewise linear interpolation of a Color parameter.
class dynamic_value_controller
{
    using value_type = Color;
    using key_map = std::map<float, value_type>;

public:
    dynamic_value_controller(const key_map& keymap)
        : key_map_(keymap)
    {
    }

    value_type get_value(float time) const
    {
        auto itUpper = key_map_.upper_bound(time + 1e-6f);
        auto itLower = itUpper;
        --itLower;

        if(itLower == key_map_.end())
        {
            return itUpper->second;
        }

        if(itUpper == key_map_.end())
        {
            return itLower->second;
        }

        float lowerTime = itLower->first;
        const auto& lowerVal = itLower->second;
        float upperTime = itUpper->first;
        const auto& upperVal = itUpper->second;

        if(lowerTime == upperTime)
        {
            return lowerVal;
        }

        return interpolate(lowerTime, lowerVal, upperTime, upperVal, time);
    };

private:
    value_type interpolate(float lowerTime,
                           const value_type& lowerVal,
                           float upperTime,
                           const value_type& upperVal,
                           float time) const
    {
        const float tt = (time - lowerTime) / (upperTime - lowerTime);
        const auto result = bx::lerp(lowerVal, upperVal, tt);
        return result;
    };

    const key_map& key_map_;
};

// HDTV rec. 709 matrix.
static constexpr float M_XYZ2RGB[] = {
    3.240479f,
    -0.969256f,
    0.055648f,
    -1.53715f,
    1.875991f,
    -0.204043f,
    -0.49853f,
    0.041556f,
    1.057311f,
};

// Converts color representation from CIE XYZ to RGB color-space.
Color xyzToRgb(const Color& xyz)
{
    Color rgb(bx::InitNone);
    rgb.x = M_XYZ2RGB[0] * xyz.x + M_XYZ2RGB[3] * xyz.y + M_XYZ2RGB[6] * xyz.z;
    rgb.y = M_XYZ2RGB[1] * xyz.x + M_XYZ2RGB[4] * xyz.y + M_XYZ2RGB[7] * xyz.z;
    rgb.z = M_XYZ2RGB[2] * xyz.x + M_XYZ2RGB[5] * xyz.y + M_XYZ2RGB[8] * xyz.z;
    return rgb;
};

// Precomputed luminance of sunlight in XYZ colorspace.
// Computed using code from Game Engine Gems, Volume One, chapter 15. Implementation based on Dr. Richard Bird model.
// This table is used for piecewise linear interpolation. Transitions from and to 0.0 at sunset and sunrise are highly
// inaccurate
static std::map<float, Color> sunLuminanceXYZTable = {
    {5.0f, {0.000000f, 0.000000f, 0.000000f}},
    {7.0f, {12.703322f, 12.989393f, 9.100411f}},
    {8.0f, {13.202644f, 13.597814f, 11.524929f}},
    {9.0f, {13.192974f, 13.597458f, 12.264488f}},
    {10.0f, {13.132943f, 13.535914f, 12.560032f}},
    {11.0f, {13.088722f, 13.489535f, 12.692996f}},
    {12.0f, {13.067827f, 13.467483f, 12.745179f}},
    {13.0f, {13.069653f, 13.469413f, 12.740822f}},
    {14.0f, {13.094319f, 13.495428f, 12.678066f}},
    {15.0f, {13.142133f, 13.545483f, 12.526785f}},
    {16.0f, {13.201734f, 13.606017f, 12.188001f}},
    {17.0f, {13.182774f, 13.572725f, 11.311157f}},
    {18.0f, {12.448635f, 12.672520f, 8.267771f}},
    {20.0f, {0.000000f, 0.000000f, 0.000000f}},
};

// Precomputed luminance of sky in the zenith point in XYZ colorspace.
// Computed using code from Game Engine Gems, Volume One, chapter 15. Implementation based on Dr. Richard Bird model.
// This table is used for piecewise linear interpolation. Day/night transitions are highly inaccurate.
// The scale of luminance change in Day/night transitions is not preserved.
// Luminance at night was increased to eliminate need the of HDR render.
static std::map<float, Color> skyLuminanceXYZTable = {
    {0.0f, bx::mul({0.308f, 0.308f, 0.411f}, 0.0f)},
    //{1.0f, {0.308f, 0.308f, 0.410f}},
    //{2.0f, {0.301f, 0.301f, 0.402f}},
    //{3.0f, {0.287f, 0.287f, 0.382f}},
    {4.0f, bx::mul({0.258f, 0.258f, 0.344f}, 0.05f)},
    {5.0f, {0.258f, 0.258f, 0.344f}},
    {7.0f, {0.962851f, 1.000000f, 1.747835f}},
    {8.0f, {0.967787f, 1.000000f, 1.776762f}},
    {9.0f, {0.970173f, 1.000000f, 1.788413f}},
    {10.0f, {0.971431f, 1.000000f, 1.794102f}},
    {11.0f, {0.972099f, 1.000000f, 1.797096f}},
    {12.0f, {0.972385f, 1.000000f, 1.798389f}},
    {13.0f, {0.972361f, 1.000000f, 1.798278f}},
    {14.0f, {0.972020f, 1.000000f, 1.796740f}},
    {15.0f, {0.971275f, 1.000000f, 1.793407f}},
    {16.0f, {0.969885f, 1.000000f, 1.787078f}},
    {17.0f, {0.967216f, 1.000000f, 1.773758f}},
    {18.0f, {0.961668f, 1.000000f, 1.739891f}},
    {20.0f, {0.264f, 0.264f, 0.352f}},
    {21.0f, bx::mul({0.264f, 0.264f, 0.352f}, 0.05f)},
    //{22.0f, {0.290f, 0.290f, 0.386f}},
    {23.0f, bx::mul({0.308f, 0.308f, 0.411f}, 0.0f)},
    {24.0f, bx::mul({0.308f, 0.308f, 0.411f}, 0.0f)},
};

// Turbidity tables. Taken from:
// A. J. Preetham, P. Shirley, and B. Smits. A Practical Analytic Model for Daylight. SIGGRAPH '99
// Coefficients correspond to xyY colorspace.
static constexpr Color ABCDE[] = {
    {-0.2592f, -0.2608f, -1.4630f},
    {0.0008f, 0.0092f, 0.4275f},
    {0.2125f, 0.2102f, 5.3251f},
    {-0.8989f, -1.6537f, -2.5771f},
    {0.0452f, 0.0529f, 0.3703f},
};

static constexpr Color ABCDE_t[] = {
    {-0.0193f, -0.0167f, 0.1787f},
    {-0.0665f, -0.0950f, -0.3554f},
    {-0.0004f, -0.0079f, -0.0227f},
    {-0.0641f, -0.0441f, 0.1206f},
    {-0.0033f, -0.0109f, -0.0670f},
};

void compute_perez_coeff(float _turbidity, float* _outPerezCoeff)
{
    const bx::Vec3 turbidity = {_turbidity, _turbidity, _turbidity};
    for(uint32_t ii = 0; ii < 5; ++ii)
    {
        const bx::Vec3 tmp = bx::mad(ABCDE_t[ii], turbidity, ABCDE[ii]);
        float* out = _outPerezCoeff + 4 * ii;
        bx::store(out, tmp);
        out[3] = 0.0f;
    }
}

float hour_of_day(math::vec3 sun_dir)
{
    // Define the ground normal vector (assuming flat and horizontal ground)
    math::vec3 normal(0.0, -1.0, 0.0);

    auto v1 = sun_dir;
    auto v2 = normal;
    auto ref = math::vec3(-1.0f, 0.0f, 0.0f);

    float angle = math::orientedAngle(v1, v2, ref);  // angle in [-pi, pi]
    angle = math::mod(angle, 2 * math::pi<float>()); // angle in [0, 2pi]
    angle = math::degrees(angle);
    // The hour angle is 0 at 6:00, 90 at 12:00, and 180 at 18:00
    // Therefore, we can use a simple linear formula to map the hour angle to the hour of day
    float hour_of_day = angle / 15;

    // Return the hour of day
    return hour_of_day;
}

} // namespace

auto atmospheric_pass_perez::init(rtti::context& ctx) -> bool
{
    auto& am = ctx.get<asset_manager>();
    auto vs_sky = am.load<gfx::shader>("engine:/data/shaders/vs_sky.sc");
    auto fs_sky = am.load<gfx::shader>("engine:/data/shaders/fs_sky.sc");

    program_ = std::make_unique<gpu_program>(vs_sky, fs_sky);

    int vertical_count = 32;
    int horizontal_count = 32;
    std::vector<gfx::screen_pos_vertex> vertices(vertical_count * horizontal_count);

    for(int i = 0; i < vertical_count; i++)
    {
        for(int j = 0; j < horizontal_count; j++)
        {
            gfx::screen_pos_vertex& v = vertices[i * vertical_count + j];
            v.x = float(j) / (horizontal_count - 1) * 2.0f - 1.0f;
            v.y = float(i) / (vertical_count - 1) * 2.0f - 1.0f;
        }
    }

    std::vector<uint16_t> indices((vertical_count - 1) * (horizontal_count - 1) * 6);

    int k = 0;
    for(int i = 0; i < vertical_count - 1; i++)
    {
        for(int j = 0; j < horizontal_count - 1; j++)
        {
            indices[k++] = (uint16_t)(j + 0 + horizontal_count * (i + 0));
            indices[k++] = (uint16_t)(j + 1 + horizontal_count * (i + 0));
            indices[k++] = (uint16_t)(j + 0 + horizontal_count * (i + 1));

            indices[k++] = (uint16_t)(j + 1 + horizontal_count * (i + 0));
            indices[k++] = (uint16_t)(j + 1 + horizontal_count * (i + 1));
            indices[k++] = (uint16_t)(j + 0 + horizontal_count * (i + 1));
        }
    }

    vb_ = std::make_unique<gfx::vertex_buffer>(
        gfx::copy(vertices.data(), sizeof(gfx::screen_pos_vertex) * vertical_count * horizontal_count),
        gfx::screen_pos_vertex::get_layout());
    ib_ = std::make_unique<gfx::index_buffer>(gfx::copy(indices.data(), sizeof(uint16_t) * k));

    sun_.update(0);

    return true;
}

auto atmospheric_pass_perez::run(gfx::frame_buffer::ptr input, const camera& camera, delta_t dt, const run_params& params)
    -> gfx::frame_buffer::ptr
{
    hour_ += time_scale_ * dt.count();
    hour_ = bx::mod(hour_, 24.0f);
    sun_.update(hour_);

    const auto& view = camera.get_view();
    const auto& proj = camera.get_projection();

    const auto surface = input.get();
    const auto output_size = surface->get_size();
    gfx::render_pass pass("atmospherics_fill");
    pass.bind(surface);
    pass.set_view_proj(view, proj);

    if(program_->is_valid())
    {
        program_->begin();
        program_->set_uniform("u_light_direction", params.light_direction);

        bx::Vec3 sun_dir(-params.light_direction.x, -params.light_direction.y, -params.light_direction.z);
        hour_ = hour_of_day(-params.light_direction);
        // APPLOG_INFO("Time Of Day {}", hour_);


        dynamic_value_controller sun_luminance_dc(sunLuminanceXYZTable);
        dynamic_value_controller sky_luminance_dc(skyLuminanceXYZTable);

        auto sunLuminanceXYZ = sun_luminance_dc.get_value(hour_);
        auto sunLuminanceRGB = xyzToRgb(sunLuminanceXYZ);

        auto skyLuminanceXYZ = sky_luminance_dc.get_value(hour_);
        auto skyLuminanceRGB = xyzToRgb(skyLuminanceXYZ);

        float exposition[4] = {0.02f, 3.0f, 0.1f, hour_};

        float perezCoeff[4 * 5];
        compute_perez_coeff(turbidity_, perezCoeff);

        program_->set_uniform("u_sunLuminance", &sunLuminanceRGB.x);
        program_->set_uniform("u_skyLuminanceXYZ", &skyLuminanceXYZ.x);
        program_->set_uniform("u_skyLuminance", &skyLuminanceRGB.x);
        program_->set_uniform("u_sunDirection", &sun_dir.x);

        program_->set_uniform("u_parameters", exposition);

        program_->set_uniform("u_perezCoeff", perezCoeff, 5);

        irect32_t rect(0, 0, irect32_t::value_type(output_size.width), irect32_t::value_type(output_size.height));
        gfx::set_scissor(rect.left, rect.top, rect.width(), rect.height());

        gfx::set_state(BGFX_STATE_WRITE_RGB | BGFX_STATE_DEPTH_TEST_EQUAL);
        gfx::set_index_buffer(ib_->native_handle());
        gfx::set_vertex_buffer(0, vb_->native_handle());
        gfx::submit(pass.id, program_->native_handle());

        gfx::set_state(BGFX_STATE_DEFAULT);
        program_->end();
    }

    return input;
}
} // namespace ace
