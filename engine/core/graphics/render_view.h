#pragma once

#include "frame_buffer.h"
#include "render_view_keys.h"
#include <base/basetypes.hpp>
#include <base/hash.hpp>
#include <chrono>
#include <functional>
#include <memory>
#include <unordered_map>

namespace gfx
{

class render_view
{
public:
	auto get_texture(const std::string& id, uint16_t _width, uint16_t _height, bool _hasMips,
					 uint16_t _numLayers, texture_format _format,
					 uint64_t _flags = get_default_rt_sampler_flags(), const memory_view* _mem = nullptr)
		-> texture::ptr;

	auto get_texture(const std::string& id, backbuffer_ratio _ratio, bool _hasMips, uint16_t _numLayers,
					 texture_format _format, uint64_t _flags = get_default_rt_sampler_flags())
		-> texture::ptr;

	auto get_texture(const std::string& id, uint16_t _width, uint16_t _height, uint16_t _depth, bool _hasMips,
					 texture_format _format, uint64_t _flags = get_default_rt_sampler_flags(),
					 const memory_view* _mem = nullptr) -> texture::ptr;

	auto get_texture(const std::string& id, uint16_t _size, bool _hasMips, uint16_t _numLayers,
					 texture_format _format, uint64_t _flags = get_default_rt_sampler_flags(),
					 const memory_view* _mem = nullptr) -> texture::ptr;

	auto get_fbo(const std::string& id, const std::vector<texture::ptr>& bind_textures) -> frame_buffer::ptr;

	auto get_depth_stencil_buffer(const usize32_t& viewport_size) -> texture::ptr;
	auto get_depth_buffer(const usize32_t& viewport_size) -> texture::ptr;
	auto get_depth_buffer(const usize32_t& viewport_size, size_t i) -> texture::ptr;

	auto get_output_buffer(const usize32_t& viewport_size) -> texture::ptr;
	auto get_output_fbo(const usize32_t& viewport_size) -> frame_buffer::ptr;
	auto get_g_buffer_fbo(const usize32_t& viewport_size) -> frame_buffer::ptr;

	void release_unused_resources();

private:
	template <typename T>
	struct entry
	{
		T item{};
		bool used_last_frame{};
	};

	using texture_storage_t = std::unordered_map<texture_key, entry<texture::ptr>>;
	using frame_buffer_storage_t = std::unordered_map<fbo_key, entry<frame_buffer::ptr>>;

	texture_storage_t textures_;
	frame_buffer_storage_t fbos_;
};
} // namespace gfx
