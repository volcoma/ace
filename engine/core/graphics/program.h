#pragma once

#include "handle_impl.h"
#include <limits>
#include <memory>
#include <unordered_map>
#include <vector>

namespace gfx
{
struct frame_buffer;
struct texture;
struct shader;
struct uniform;

struct program : public handle_impl<program, program_handle>
{
	using uniform_ptr = std::shared_ptr<uniform>;
	//-----------------------------------------------------------------------------
	//  Name : program ()
	/// <summary>
	///
	///
	///
	/// </summary>
	//-----------------------------------------------------------------------------
	program() = default;

	//-----------------------------------------------------------------------------
	//  Name : program ()
	/// <summary>
	///
	///
	///
	/// </summary>
	//-----------------------------------------------------------------------------
	program(const std::shared_ptr<shader>& compute_shader);

	//-----------------------------------------------------------------------------
	//  Name : program ()
	/// <summary>
	///
	///
	///
	/// </summary>
	//-----------------------------------------------------------------------------
	program(const std::shared_ptr<shader>& vertex_shader, const std::shared_ptr<shader>& fragment_shader);

	//-----------------------------------------------------------------------------
	//  Name : set_texture ()
	/// <summary>
	///
	///
	///
	/// </summary>
	//-----------------------------------------------------------------------------
	void set_texture(uint8_t _stage, const std::string& _sampler, gfx::frame_buffer* _handle,
					 uint8_t _attachment = 0, uint32_t _flags = std::numeric_limits<uint32_t>::max());

	//-----------------------------------------------------------------------------
	//  Name : set_texture ()
	/// <summary>
	///
	///
	///
	/// </summary>
	//-----------------------------------------------------------------------------
	void set_texture(uint8_t _stage, const std::string& _sampler, gfx::texture* _texture,
					 uint32_t _flags = std::numeric_limits<uint32_t>::max());

	//-----------------------------------------------------------------------------
	//  Name : set_uniform ()
	/// <summary>
	///
	///
	///
	/// </summary>
	//-----------------------------------------------------------------------------
	void set_uniform(const std::string& _name, const void* _value, uint16_t _num = 1);

	//-----------------------------------------------------------------------------
	//  Name : get_uniform ()
	/// <summary>
	///
	///
	///
	/// </summary>
	//-----------------------------------------------------------------------------
	auto get_uniform(const std::string& _name, bool texture = false) -> uniform_ptr;

	/// All uniforms for this program.
	std::unordered_map<std::string, uniform_ptr> uniforms;
};
} // namespace gfx
