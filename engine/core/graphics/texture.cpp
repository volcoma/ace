#include "texture.h"
#include "utils/bgfx_utils.h"
namespace gfx
{

texture::texture(const char* _path,
                 std::uint64_t _flags,
                 std::uint8_t _skip /*= 0 */,
                 texture_info* _info /*= nullptr*/)
{
    handle_ = loadTexture(_path, _flags, _skip, &info);

    if(_info != nullptr)
    {
        *_info = info;
    }

    flags = _flags;
}

texture::texture(std::uint16_t _width,
                 std::uint16_t _height,
                 bool _hasMips,
                 std::uint16_t _numLayers,
                 texture_format _format,
                 std::uint64_t _flags /*= BGFX_TEXTURE_NONE */,
                 const memory_view* _mem /*= nullptr */)
    : flags(_flags)
{
    handle_ = create_texture_2d(_width, _height, _hasMips, _numLayers, _format, _flags, _mem);

    calc_texture_size(info, _width, _height, 1, false, _hasMips, _numLayers, _format);


}

texture::texture(std::uint16_t _width,
                 std::uint16_t _height,
                 std::uint16_t _depth,
                 bool _hasMips,
                 texture_format _format,
                 std::uint64_t _flags /*= BGFX_TEXTURE_NONE */,
                 const memory_view* _mem /*= nullptr */)
    : flags(_flags)
{
    handle_ = create_texture_3d(_width, _height, _depth, _hasMips, _format, _flags, _mem);

    calc_texture_size(info, _width, _height, _depth, false, _hasMips, 1, _format);


}

texture::texture(std::uint16_t _size,
                 bool _hasMips,
                 std::uint16_t _numLayers,
                 texture_format _format,
                 std::uint64_t _flags /*= BGFX_TEXTURE_NONE */,
                 const memory_view* _mem /*= nullptr */)
    : flags(_flags)
{
    handle_ = create_texture_cube(_size, _hasMips, _numLayers, _format, _flags, _mem);

    calc_texture_size(info, _size, _size, _size, false, _hasMips, _numLayers, _format);


}

auto texture::get_size() const -> usize32_t
{
    return {static_cast<std::uint32_t>(info.width), static_cast<std::uint32_t>(info.height)};
}

auto texture::is_render_target() const -> bool
{
    return 0 != (flags & BGFX_TEXTURE_RT_MASK);
}
} // namespace gfx
