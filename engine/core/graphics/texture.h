#pragma once

#include "handle_impl.h"
#include <base/basetypes.hpp>
#include <memory>

namespace gfx
{
struct texture : public handle_impl<texture, texture_handle>
{
    //-----------------------------------------------------------------------------
    //  Name : Texture ()
    /// <summary>
    ///
    ///
    ///
    /// </summary>
    //-----------------------------------------------------------------------------
    texture() = default;

    texture(const char* _path,
            std::uint64_t _flags = BGFX_TEXTURE_NONE | BGFX_SAMPLER_NONE,
            std::uint8_t _skip = 0,
            texture_info* _info = nullptr);

    //-----------------------------------------------------------------------------
    //  Name : Texture ()
    /// <summary>
    ///
    ///
    ///
    /// </summary>
    //-----------------------------------------------------------------------------
    texture(std::uint16_t _width,
            std::uint16_t _height,
            bool _hasMips,
            std::uint16_t _numLayers,
            texture_format _format,
            std::uint64_t _flags = BGFX_TEXTURE_NONE | BGFX_SAMPLER_NONE,
            const memory_view* _mem = nullptr);


    //-----------------------------------------------------------------------------
    //  Name : Texture ()
    /// <summary>
    ///
    ///
    ///
    /// </summary>
    //-----------------------------------------------------------------------------
    texture(std::uint16_t _width,
            std::uint16_t _height,
            std::uint16_t _depth,
            bool _hasMips,
            texture_format _format,
            std::uint64_t _flags = BGFX_TEXTURE_NONE | BGFX_SAMPLER_NONE,
            const memory_view* _mem = nullptr);

    //-----------------------------------------------------------------------------
    //  Name : Texture ()
    /// <summary>
    ///
    ///
    ///
    /// </summary>
    //-----------------------------------------------------------------------------
    texture(std::uint16_t _size,
            bool _hasMips,
            std::uint16_t _numLayers,
            texture_format _format,
            std::uint64_t _flags = BGFX_TEXTURE_NONE | BGFX_SAMPLER_NONE,
            const memory_view* _mem = nullptr);

    //-----------------------------------------------------------------------------
    //  Name : get_size ()
    /// <summary>
    ///
    ///
    ///
    /// </summary>
    //-----------------------------------------------------------------------------
    auto get_size() const -> usize32_t;

    //-----------------------------------------------------------------------------
    //  Name : is_render_target ()
    /// <summary>
    ///
    ///
    ///
    /// </summary>
    //-----------------------------------------------------------------------------
    auto is_render_target() const -> bool;

    /// Texture detail info.
    texture_info info{};
    /// Creation flags.
    std::uint64_t flags = BGFX_TEXTURE_NONE;
};
} // namespace gfx
