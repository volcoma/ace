#include "program.h"
#include "frame_buffer.h"
#include "shader.h"
#include "texture.h"
#include "uniform.h"

namespace gfx
{
program::program(const std::shared_ptr<shader>& compute_shader)
{
	if(compute_shader)
	{
		handle = create_program(compute_shader->native_handle());

		for(auto& uniform : compute_shader->uniforms)
		{
			uniforms[uniform->info.name] = uniform;
		}
	}
}

program::program(const std::shared_ptr<shader>& vertex_shader, const std::shared_ptr<shader>& fragment_shader)
{
	if(vertex_shader && fragment_shader)
	{
		handle = create_program(vertex_shader->native_handle(), fragment_shader->native_handle());

		for(auto& uniform : vertex_shader->uniforms)
		{
			uniforms[uniform->info.name] = uniform;
		}

		for(auto& uniform : fragment_shader->uniforms)
		{
			uniforms[uniform->info.name] = uniform;
		}
	}
}

void program::set_texture(uint8_t _stage, const std::string& _sampler, gfx::frame_buffer* frameBuffer,
						  uint8_t _attachment /*= 0 */,
						  uint32_t _flags /*= std::numeric_limits<uint32_t>::max()*/)
{
	if(frameBuffer == nullptr)
	{
		return;
	}

	gfx::set_texture(_stage, get_uniform(_sampler, true)->native_handle(),
					 frameBuffer->get_texture(_attachment)->native_handle(), _flags);
}

void program::set_texture(uint8_t _stage, const std::string& _sampler, gfx::texture* _texture,
						  uint32_t _flags /*= std::numeric_limits<uint32_t>::max()*/)
{
	if(_texture == nullptr)
	{
		return;
	}

	gfx::set_texture(_stage, get_uniform(_sampler, true)->native_handle(), _texture->native_handle(), _flags);
}

void program::set_uniform(const std::string& _name, const void* _value, uint16_t _num)
{
	auto uniform = get_uniform(_name);

	if(uniform)
	{
		gfx::set_uniform(uniform->native_handle(), _value, _num);
	}
}

auto program::get_uniform(const std::string& _name, bool texture) -> uniform_ptr
{
	uniform_ptr uniform;
	auto it = uniforms.find(_name);
	if(it != uniforms.end())
	{
		uniform = it->second;
	}
	else
	{
		if(texture)
		{
			uniform = std::make_shared<gfx::uniform>(_name, gfx::uniform_type::Sampler, 1);
			uniforms[_name] = uniform;
		}
	}

	return uniform;
}
} // namespace gfx
