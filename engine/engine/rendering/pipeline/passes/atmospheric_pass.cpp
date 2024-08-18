#include "atmospheric_pass.h"
#include <engine/assets/asset_manager.h>
#include <graphics/render_pass.h>
#include <graphics/texture.h>

namespace ace
{

namespace
{
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

// Define the static Perez coefficients
static constexpr bx::Vec3 ABCDE[] = {
    bx::Vec3{-0.2592f, -0.2608f, -1.4630f},
    bx::Vec3{0.0008f, 0.0092f, 0.4275f},
    bx::Vec3{0.2125f, 0.2102f, 5.3251f},
    bx::Vec3{-0.8989f, -1.6537f, -2.5771f},
    bx::Vec3{0.0452f, 0.0529f, 0.3703f},
};

static constexpr bx::Vec3 ABCDE_t[] = {
    bx::Vec3{-0.0193f, -0.0167f, 0.1787f},
    bx::Vec3{-0.0665f, -0.0950f, -0.3554f},
    bx::Vec3{-0.0004f, -0.0079f, -0.0227f},
    bx::Vec3{-0.0641f, -0.0441f, 0.1206f},
    bx::Vec3{-0.0033f, -0.0109f, -0.0670f},
};

// Function to compute Perez coefficients based on turbidity
void compute_perez_coeff(float turbidity, float* outPerezCoeff)
{
    const bx::Vec3 turbidityVec = {turbidity, turbidity, turbidity};
    for(uint32_t ii = 0; ii < 5; ++ii)
    {
        const bx::Vec3 tmp = bx::mad(ABCDE_t[ii], turbidityVec, ABCDE[ii]);
        float* out = outPerezCoeff + 4 * ii;
        bx::store(out, tmp);
        out[3] = 0.0f; // Set alpha to 0
    }
}

} // namespace

auto atmospheric_pass::init(rtti::context& ctx) -> bool
{
    auto& am = ctx.get<asset_manager>();
    auto vs_clip_quad_ex = am.get_asset<gfx::shader>("engine:/data/shaders/atmospherics/vs_atmospherics.sc");
    auto fs_atmospherics = am.get_asset<gfx::shader>("engine:/data/shaders/atmospherics/fs_atmospherics.sc");

    atmospheric_program_.program = std::make_unique<gpu_program>(vs_clip_quad_ex, fs_atmospherics);
    atmospheric_program_.cache_uniforms();

    return true;
}

void atmospheric_pass::run(gfx::frame_buffer::ptr input, const camera& camera, delta_t dt, const run_params& params)
{
    const auto& view = camera.get_view();
    const auto& proj = camera.get_projection();

    const auto surface = input.get();
    const auto output_size = surface->get_size();
    gfx::render_pass pass("atmospherics_fill");
    pass.bind(surface);
    pass.set_view_proj(view, proj);

    auto hour = hour_of_day(-params.light_direction);
    // APPLOG_INFO("Time Of Day {}", hour);

    if(atmospheric_program_.program->is_valid())
    {
        atmospheric_program_.program->begin();

        auto turbidity = params.turbidity;

        // Interpolation factor based on turbidity range
        float t = (turbidity - 1.9f) / (10.0f - 1.9f);

        // Define clear and hazy atmospheric conditions
        math::vec3 kr_clear(0.12867780436772762f, 0.2478442963618773f, 0.6216065586417131f);
        math::vec3 kr_hazy(0.05f, 0.1f, 0.25f);
        math::vec3 u_kr = math::mix(kr_clear, kr_hazy, t);

        float rayleigh_brightness_clear = 9.0f;
        float rayleigh_brightness_hazy = 5.0f; // Example value, adjust as needed
        float u_rayleigh_brightness = math::mix(rayleigh_brightness_clear, rayleigh_brightness_hazy, t);

        float mie_brightness_clear = 0.1f;
        float mie_brightness_hazy = 0.5f; // Example value, adjust as needed
        float u_mie_brightness = math::mix(mie_brightness_clear, mie_brightness_hazy, t);

        float spot_brightness_clear = 10.0f;
        float spot_brightness_hazy = 5.0f; // Example value, adjust as needed
        float u_spot_brightness = math::mix(spot_brightness_clear, spot_brightness_hazy, t);

        float spot_distance_clear = 300.0f;
        float spot_distance_hazy = 100.0f; // Example value, adjust as needed
        float u_spot_distance = math::mix(spot_distance_clear, spot_distance_hazy, t);

        float scatter_strength_clear = 0.078f;
        float scatter_strength_hazy = 0.15f;
        float u_scatter_strength = math::mix(scatter_strength_clear, scatter_strength_hazy, t);

        float rayleigh_strength_clear = 0.139f;
        float rayleigh_strength_hazy = 0.05f;
        float u_rayleigh_strength = math::mix(rayleigh_strength_clear, rayleigh_strength_hazy, t);

        float mie_strength_clear = 0.264f;
        float mie_strength_hazy = 0.5f;
        float u_mie_strength = math::mix(mie_strength_clear, mie_strength_hazy, t);

        float rayleigh_collection_power_clear = 0.81f;
        float rayleigh_collection_power_hazy = 0.6f; // Example value, adjust as needed
        float u_rayleigh_collection_power =
            math::mix(rayleigh_collection_power_clear, rayleigh_collection_power_hazy, t);

        float mie_collection_power_clear = 0.39f;
        float mie_collection_power_hazy = 0.6f; // Example value, adjust as needed
        float u_mie_collection_power = math::mix(mie_collection_power_clear, mie_collection_power_hazy, t);

        float mie_distribution_clear = 0.53f;
        float mie_distribution_hazy = 0.7f;
        float u_mie_distribution = math::mix(mie_distribution_clear, mie_distribution_hazy, t);

        float intensity_clear = 1.8f;
        float intensity_hazy = 0.8f;
        float u_intensity = math::mix(intensity_clear, intensity_hazy, t);

        math::vec4 u_parameters(params.light_direction, hour);

        math::vec4 u_kr_and_intensity(u_kr, u_intensity);
        math::vec4 u_turbidity_parameters1(u_rayleigh_strength, u_mie_strength, u_mie_distribution, u_scatter_strength);
        math::vec4 u_turbidity_parameters2(u_rayleigh_brightness, u_mie_brightness, u_spot_brightness, u_spot_distance);
        math::vec4 u_turbidity_parameters3(u_rayleigh_collection_power, u_mie_collection_power, 0.0f, 0.0f);

        gfx::set_uniform(atmospheric_program_.u_parameters, u_parameters);
        gfx::set_uniform(atmospheric_program_.u_kr_and_intensity, u_kr_and_intensity);
        gfx::set_uniform(atmospheric_program_.u_turbidity_parameters1, u_turbidity_parameters1);
        gfx::set_uniform(atmospheric_program_.u_turbidity_parameters2, u_turbidity_parameters2);
        gfx::set_uniform(atmospheric_program_.u_turbidity_parameters3, u_turbidity_parameters3);

        irect32_t rect(0, 0, irect32_t::value_type(output_size.width), irect32_t::value_type(output_size.height));
        gfx::set_scissor(rect.left, rect.top, rect.width(), rect.height());
        auto topology = gfx::clip_quad(1.0f);

        gfx::set_state(topology | BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_DEPTH_TEST_EQUAL);

        gfx::submit(pass.id, atmospheric_program_.program->native_handle());
        gfx::set_state(BGFX_STATE_DEFAULT);
        atmospheric_program_.program->end();
    }

    gfx::discard();
}
} // namespace ace
