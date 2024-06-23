#include "material.h"

#include <graphics/texture.h>
#include <graphics/uniform.h>

namespace ace
{

auto material::default_color_map() -> asset_handle<gfx::texture>&
{
    static asset_handle<gfx::texture> texture;
    return texture;
}

auto material::default_normal_map() -> asset_handle<gfx::texture>&
{
    static asset_handle<gfx::texture> texture;
    return texture;
}

auto material::get_render_states(bool apply_cull, bool depth_write, bool depth_test) const -> uint64_t
{
    // Set render states.
    uint64_t states = 0 | BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_MSAA;

    if(depth_write)
        states |= BGFX_STATE_WRITE_Z;

    if(depth_test)
        states |= BGFX_STATE_DEPTH_TEST_LESS;

    if(apply_cull)
    {
        auto cullType = get_cull_type();
        if(cullType == cull_type::counter_clockwise)
            states |= BGFX_STATE_CULL_CCW;
        if(cullType == cull_type::clockwise)
            states |= BGFX_STATE_CULL_CW;
    }

    return states;
}

} // namespace ace
