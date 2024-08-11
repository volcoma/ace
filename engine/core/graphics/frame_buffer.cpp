#include "frame_buffer.h"

namespace gfx
{
namespace
{
}
frame_buffer::frame_buffer(std::uint16_t _width,
                           std::uint16_t _height,
                           texture_format _format,
                           std::uint32_t _textureFlags)
    : frame_buffer(std::vector<texture::ptr>{
          std::make_shared<texture>(_width, _height, false, 1, _format, _textureFlags),
      })
{
}

frame_buffer::frame_buffer(const std::vector<texture::ptr>& textures)
{
    populate(textures);
}

frame_buffer::frame_buffer(const std::vector<fbo_attachment>& textures)
{
    populate(textures);
}

frame_buffer::frame_buffer(void* _nwh,
                           uint16_t _width,
                           uint16_t _height,
                           texture_format _format,
                           texture_format _depth_format)
{
    handle_ = create_frame_buffer(_nwh, _width, _height, _format, _depth_format);

    cached_size_ = {_width, _height};
}

void frame_buffer::populate(const std::vector<texture::ptr>& textures)
{
    std::vector<fbo_attachment> attachments;
    attachments.reserve(textures.size());
    for(const auto& tex : textures)
    {
        attachments.emplace_back();
        auto& back = attachments.back();
        back.texture = tex;
    }

    populate(attachments);
}

void frame_buffer::populate(const std::vector<fbo_attachment>& textures)
{
    std::vector<attachment> buffer;
    buffer.reserve(textures.size());

    usize32_t size = {0, 0};
    for(const auto& tex : textures)
    {
        size = {tex.texture->info.width, tex.texture->info.height};

        buffer.emplace_back();
        auto& att = buffer.back();
        att.init(tex.texture->native_handle(), access::Write, tex.layer, 1, tex.mip);
    }
    textures_ = textures;

    handle_ = create_frame_buffer(static_cast<std::uint8_t>(buffer.size()), buffer.data(), false);
    cached_size_ = size;
}

auto frame_buffer::get_size() const -> usize32_t
{
    return cached_size_;
}

auto frame_buffer::get_attachment(std::uint32_t index) const -> const fbo_attachment&
{
    return textures_[index];
}

auto frame_buffer::get_texture(uint32_t index) const -> const texture::ptr&
{
    return get_attachment(index).texture;
}

auto frame_buffer::get_attachment_count() const -> size_t
{
    return textures_.size();
}
} // namespace gfx
