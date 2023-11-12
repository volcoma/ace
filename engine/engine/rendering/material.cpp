#include "material.h"

#include <graphics/texture.h>
#include <graphics/uniform.h>

namespace ace
{

asset_handle<gfx::texture>& material::default_color_map()
{
    static asset_handle<gfx::texture> texture;
    return texture;
}

asset_handle<gfx::texture>& material::default_normal_map()
{
    static asset_handle<gfx::texture> texture;
    return texture;
}

void material::set_texture(std::uint8_t _stage,
                           const std::string& _sampler,
                           gfx::texture* _texture,
                           std::uint32_t _flags /*= std::numeric_limits<std::uint32_t>::max()*/)
{
    get_program().set_texture(_stage, _sampler, _texture, _flags);
}

void material::set_texture(std::uint8_t _stage,
                           const std::string& _sampler,
                           gfx::frame_buffer* _handle,
                           uint8_t _attachment /*= 0 */,
                           std::uint32_t _flags /*= std::numeric_limits<std::uint32_t>::max()*/)
{
    get_program().set_texture(_stage, _sampler, _handle, _attachment, _flags);
}

void material::set_uniform(const std::string& _name, const void* _value, std::uint16_t _num /*= 1*/)
{
    get_program().set_uniform(_name, _value, _num);
}

std::uint64_t material::get_render_states(bool apply_cull, bool depth_write, bool depth_test) const
{
    // Set render states.
    std::uint64_t states = 0 | BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_MSAA;

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

gpu_program& standard_material::program()
{
    static gpu_program program;
    return program;
}
gpu_program& standard_material::program_skinned()
{
    static gpu_program program;
    return program;
}

const gpu_program& standard_material::get_program() const
{
    return skinned ? program_skinned() : program();
}

gpu_program& standard_material::get_program()
{
    return skinned ? program_skinned() : program();
}

void standard_material::submit()
{
    if(!is_valid())
        return;

    get_program().set_uniform("u_base_color", &base_color_);
    get_program().set_uniform("u_subsurface_color", &subsurface_color_);
    get_program().set_uniform("u_emissive_color", &emissive_color_);
    get_program().set_uniform("u_surface_data", &surface_data_);
    get_program().set_uniform("u_tiling", &tiling_);
    get_program().set_uniform("u_dither_threshold", &dither_threshold_);

    const auto& color_map = maps_["color"];
    const auto& normal_map = maps_["normal"];
    const auto& roughness_map = maps_["roughness"];
    const auto& metalness_map = maps_["metalness"];
    const auto& ao_map = maps_["ao"];

    const auto& albedo = color_map ? color_map : default_color_map();
    const auto& normal = normal_map ? normal_map : default_normal_map();
    const auto& roughness = roughness_map ? roughness_map : default_color_map();
    const auto& metalness = metalness_map ? metalness_map : default_color_map();
    const auto& ao = ao_map ? ao_map : default_color_map();

    get_program().set_texture(0, "s_tex_color", &albedo.get());
    get_program().set_texture(1, "s_tex_normal", &normal.get());
    get_program().set_texture(2, "s_tex_roughness", &roughness.get());
    get_program().set_texture(3, "s_tex_metalness", &metalness.get());
    get_program().set_texture(4, "s_tex_ao", &ao.get());
}
} // namespace ace
