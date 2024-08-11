#pragma once

#include <engine/rendering/camera.h>
#include <engine/rendering/gpu_program.h>

namespace ace
{

enum class tonemapping_method : uint8_t
{
    none = 0,
    exponential,
    reinhard,
    reinhard_lum,
    hable,
    duiker,
    aces,
    aces_lum,
    filmic
};

class tonemapping_pass
{
public:
    struct run_params
    {
        gfx::frame_buffer::ptr input;
        gfx::frame_buffer::ptr output;
        float exposure = 1.0f;
        tonemapping_method method = tonemapping_method::aces;
    };

    auto init(rtti::context& ctx) -> bool;
    void run(const run_params& params);

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
