#pragma once

#include <engine/rendering/camera.h>
#include <engine/rendering/gpu_program.h>

namespace ace
{

class tonemapping_pass
{
public:
    struct run_params
    {

    };

    auto init(rtti::context& ctx) -> bool;
    void run(gfx::frame_buffer::ptr input, std::shared_ptr<gfx::frame_buffer> output);

private:
    struct tonemapping_program : uniforms_cache
    {
        void cache_uniforms()
        {
            cache_uniform(program.get(), u_tonemapping, "u_tonemapping");
            cache_uniform(program.get(), s_input, "s_input");
        }

        gfx::program::uniform_ptr u_tonemapping;
        gfx::program::uniform_ptr s_input;

        std::unique_ptr<gpu_program> program;

    } tonemapping_program_;
};
} // namespace ace
