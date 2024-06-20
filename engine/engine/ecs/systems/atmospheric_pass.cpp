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
} // namespace

auto atmospheric_pass::init(rtti::context& ctx) -> bool
{
    auto& am = ctx.get<asset_manager>();
    auto vs_clip_quad_ex = am.get_asset<gfx::shader>("engine:/data/shaders/vs_atmospherics.sc");
    auto fs_atmospherics = am.get_asset<gfx::shader>("engine:/data/shaders/fs_atmospherics.sc");

    atmospheric_program_.program = std::make_unique<gpu_program>(vs_clip_quad_ex, fs_atmospherics);
    atmospheric_program_.cache_uniforms();


    return true;
}

auto atmospheric_pass::run(gfx::frame_buffer::ptr input, const camera& camera, delta_t dt, const run_params& params)
    -> gfx::frame_buffer::ptr
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

        math::vec4 parameters(params.light_direction, hour);
        gfx::set_uniform(atmospheric_program_.u_parameters, parameters);

        irect32_t rect(0, 0, irect32_t::value_type(output_size.width), irect32_t::value_type(output_size.height));
        gfx::set_scissor(rect.left, rect.top, rect.width(), rect.height());
        auto topology = gfx::clip_quad(1.0f);

        gfx::set_state(topology | BGFX_STATE_WRITE_RGB | BGFX_STATE_DEPTH_TEST_EQUAL);

        gfx::submit(pass.id, atmospheric_program_.program->native_handle());
        gfx::set_state(BGFX_STATE_DEFAULT);
        atmospheric_program_.program->end();
    }

    return input;
}
} // namespace ace
