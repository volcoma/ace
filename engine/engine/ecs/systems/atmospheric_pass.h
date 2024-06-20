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
    };

    auto init(rtti::context& ctx) -> bool;
    auto run(gfx::frame_buffer::ptr input, const camera& camera, delta_t dt, const run_params& params)
        -> gfx::frame_buffer::ptr;

private:

    struct atmospheric_program : uniforms_cache
    {
        void cache_uniforms()
        {
            cache_uniform(program.get(), u_parameters, "u_parameters");
        }

        gfx::program::uniform_ptr u_parameters;
        std::unique_ptr<gpu_program> program;

    } atmospheric_program_;
};
} // namespace ace
