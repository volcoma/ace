#pragma once

#include <engine/rendering/camera.h>
#include <engine/rendering/gpu_program.h>

namespace ace
{

class atmospheric_pass
{
public:
    struct run_params
    {
        math::vec3 light_direction = math::normalize(math::vec3(0.2f, -0.8f, 1.0f));

        // [1.9 - 10.0f]
        float turbidity = 1.9f;
    };

    auto init(rtti::context& ctx) -> bool;
    void run(gfx::frame_buffer::ptr input, const camera& camera, delta_t dt, const run_params& params);

private:
    struct atmospheric_program : uniforms_cache
    {
        void cache_uniforms()
        {
            cache_uniform(program.get(), u_parameters, "u_parameters");
            cache_uniform(program.get(), u_kr_and_intensity, "u_kr_and_intensity");
            cache_uniform(program.get(), u_turbidity_parameters1, "u_turbidity_parameters1");
            cache_uniform(program.get(), u_turbidity_parameters2, "u_turbidity_parameters2");
            cache_uniform(program.get(), u_turbidity_parameters3, "u_turbidity_parameters3");

        }

        gfx::program::uniform_ptr u_parameters;
        gfx::program::uniform_ptr u_kr_and_intensity;
        gfx::program::uniform_ptr u_turbidity_parameters1;
        gfx::program::uniform_ptr u_turbidity_parameters2;
        gfx::program::uniform_ptr u_turbidity_parameters3;


        std::unique_ptr<gpu_program> program;

    } atmospheric_program_;
};
} // namespace ace
