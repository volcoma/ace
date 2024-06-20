#include "program.h"
#include "frame_buffer.h"
#include "shader.h"
#include "texture.h"
#include "uniform.h"

namespace gfx
{
program::program(const shader& compute_shader)
{
    if(compute_shader.is_valid())
    {
        handle = create_program(compute_shader.native_handle());

        for(auto& uniform : compute_shader.uniforms)
        {
            uniforms[uniform->info.name] = uniform;
        }
    }

    textures_uniforms.fill(nullptr);

}

program::program(const shader& vertex_shader, const shader& fragment_shader)
{
    if(vertex_shader.is_valid() && fragment_shader.is_valid())
    {
        handle = create_program(vertex_shader.native_handle(), fragment_shader.native_handle());

        for(auto& uniform : vertex_shader.uniforms)
        {
            uniforms[uniform->info.name] = uniform;
        }

        for(auto& uniform : fragment_shader.uniforms)
        {
            uniforms[uniform->info.name] = uniform;
        }
    }

    textures_uniforms.fill(nullptr);

}

void program::set_texture(uint8_t _stage,
                          const hpp::string_view& _sampler,
                          const gfx::frame_buffer* frameBuffer,
                          uint8_t _attachment /*= 0 */,
                          uint32_t _flags /*= std::numeric_limits<uint32_t>::max()*/)
{
    if(frameBuffer == nullptr)
    {
        return;
    }

    auto uniform = get_uniform(_sampler, _stage);

    if(!uniform)
    {
        return;
    }

    gfx::set_texture(_stage, uniform->native_handle(), frameBuffer->get_texture(_attachment)->native_handle(), _flags);
}

void program::set_texture(uint8_t _stage,
                          const hpp::string_view& _sampler,
                          const gfx::texture* _texture,
                          uint32_t _flags /*= std::numeric_limits<uint32_t>::max()*/)
{
    if(_texture == nullptr)
    {
        return;
    }

    auto uniform = get_uniform(_sampler, _stage);

    if(!uniform)
    {
        return;
    }

    gfx::set_texture(_stage, uniform->native_handle(), _texture->native_handle(), _flags);
}

void program::set_uniform(const hpp::string_view& _name, const void* _value, uint16_t _num)
{
    auto uniform = get_uniform(_name);

    if(uniform)
    {
        gfx::set_uniform(uniform->native_handle(), _value, _num);
    }
}

auto program::get_uniform(const hpp::string_view& _name, uint8_t stage) -> uniform_ptr
{
    uniform_ptr uniform = nullptr;

    bool is_texture = stage != uint8_t(-1) && stage < textures_uniforms.size();

    if(is_texture)
    {
        uniform = textures_uniforms[stage];
    }

    if(!uniform)
    {
        auto it = uniforms.find(_name);

        if(it != uniforms.end())
        {
            uniform = it->second;

            if(is_texture)
            {
                textures_uniforms[stage] = uniform;
            }
        }
    }

    return uniform;
}
} // namespace gfx
