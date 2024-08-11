#pragma once

#include "frame_buffer.h"
#include <base/basetypes.hpp>
#include <base/hash.hpp>
#include <functional>
#include <hpp/string_view.hpp>
#include <map>

namespace gfx
{


class render_view
{
public:
    auto fbo_get_or_emplace(const hpp::string_view& id) -> frame_buffer::ptr&;
    auto fbo_get(const hpp::string_view& id) const -> const frame_buffer::ptr&;
    auto fbo_safe_get(const hpp::string_view& id) const -> const frame_buffer::ptr&;

    auto tex_get_or_emplace(const hpp::string_view& id) -> texture::ptr&;
    auto tex_get(const hpp::string_view& id) const -> const texture::ptr&;
    auto tex_safe_get(const hpp::string_view& id) const -> const texture::ptr&;

private:
    std::map<std::string, texture::ptr, std::less<>> textures_;
    std::map<std::string, frame_buffer::ptr, std::less<>> fbos_;
};

} // namespace gfx
