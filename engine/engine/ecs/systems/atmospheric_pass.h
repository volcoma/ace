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
    auto run(gfx::frame_buffer::ptr input, camera& camera, delta_t dt, const run_params& params)
        -> gfx::frame_buffer::ptr;

private:
    /// Program that is responsible for rendering.
    std::unique_ptr<gpu_program> program_;
};
} // namespace ace
