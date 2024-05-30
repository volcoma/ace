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

void pbr_material::submit(gpu_program* program) const
{
    if(!program)
        return;

    const auto& color_map = get_color_map();
    const auto& normal_map = get_normal_map();
    const auto& roughness_map = get_roughness_map();
    const auto& metalness_map = get_metalness_map();
    const auto& ao_map = get_ao_map();
    const auto& emissive_map = get_emissive_map();

    const auto& albedo = color_map ? color_map : default_color_map();
    const auto& normal = normal_map ? normal_map : default_normal_map();
    const auto& roughness = roughness_map ? roughness_map : default_color_map();
    const auto& metalness = metalness_map ? metalness_map : default_color_map();
    const auto& ao = ao_map ? ao_map : default_color_map();
    const auto& emissive = emissive_map ? emissive_map : default_color_map();

    program->set_texture(0, "s_tex_color", albedo.get_ptr().get());
    program->set_texture(1, "s_tex_normal", normal.get_ptr().get());
    program->set_texture(2, "s_tex_roughness", roughness.get_ptr().get());
    program->set_texture(3, "s_tex_metalness", metalness.get_ptr().get());
    program->set_texture(4, "s_tex_ao", ao.get_ptr().get());
    program->set_texture(5, "s_tex_emissive", emissive.get_ptr().get());

    program->set_uniform("u_base_color", &base_color_);
    program->set_uniform("u_subsurface_color", &subsurface_color_);
    program->set_uniform("u_emissive_color", &emissive_color_);
    program->set_uniform("u_surface_data", &surface_data_);
    program->set_uniform("u_tiling", &tiling_);
    program->set_uniform("u_dither_threshold", &dither_threshold_);

    math::vec4 surface_data2{};

    if(metalness == roughness)
    {
        surface_data2[0] = 1.0f;
    }

    program->set_uniform("u_surface_data2", &surface_data2);
}

auto pbr_material::get_map(const hpp::string_view& id) const -> const asset_handle<gfx::texture>&
{
    auto it = maps_.find(id);
    if(it != std::end(maps_))
    {
        return it->second;
    }

    return asset_handle<gfx::texture>::get_empty();
}

auto pbr_material::get_map(const hpp::string_view& id) -> asset_handle<gfx::texture>&
{
    auto it = maps_.find(id);
    if(it != std::end(maps_))
    {
        return it->second;
    }

    return maps_[std::string(id)];
}

} // namespace ace
