#include "format.h"

namespace gfx
{

auto get_best_float_format(std::uint16_t type_flags,
                           std::uint32_t search_flags,
                           bool requires_alpha,
                           bool accept_padding,
                           bool accept_half,
                           bool accept_full) -> texture_format
{
    if(search_flags & format_search_flags::four_channels)
    {
        if(accept_full && is_format_supported(type_flags, texture_format::RGBA32F))
            return texture_format::RGBA32F;
        if(accept_half && is_format_supported(type_flags, texture_format::RGBA16F))
            return texture_format::RGBA16F;
    }
    else if(search_flags & format_search_flags::two_channels)
    {
        if(!requires_alpha)
        {
            if(accept_full && is_format_supported(type_flags, texture_format::RG32F))
                return texture_format::RG32F;
            if(accept_half && is_format_supported(type_flags, texture_format::RG16F))
                return texture_format::RG16F;
            if(accept_padding && accept_half && is_format_supported(type_flags, texture_format::RGBA16F))
                return texture_format::RGBA16F;
            if(accept_padding && accept_full && is_format_supported(type_flags, texture_format::RGBA32F))
                return texture_format::RGBA32F;
        }
        else
        {
            if(accept_padding && accept_half && is_format_supported(type_flags, texture_format::RGBA16F))
                return texture_format::RGBA16F;
            if(accept_padding && accept_full && is_format_supported(type_flags, texture_format::RGBA32F))
                return texture_format::RGBA32F;
        }
    }
    else if(search_flags & format_search_flags::one_channel)
    {
        if(!requires_alpha)
        {
            if(accept_full && is_format_supported(type_flags, texture_format::R32F))
                return texture_format::R32F;
            if(accept_half && is_format_supported(type_flags, texture_format::R16F))
                return texture_format::R16F;
            if(accept_padding && accept_half && is_format_supported(type_flags, texture_format::RG16F))
                return texture_format::RG16F;
            if(accept_padding && accept_full && is_format_supported(type_flags, texture_format::RG32F))
                return texture_format::RG32F;
            if(accept_padding && accept_half && is_format_supported(type_flags, texture_format::RGBA16F))
                return texture_format::RGBA16F;
            if(accept_padding && accept_full && is_format_supported(type_flags, texture_format::RGBA32F))
                return texture_format::RGBA32F;
        }
        else
        {
            if(accept_padding && accept_half && is_format_supported(type_flags, texture_format::RGBA16F))
                return texture_format::RGBA16F;
            if(accept_padding && accept_full && is_format_supported(type_flags, texture_format::RGBA32F))
                return texture_format::RGBA32F;
        }
    }

    return texture_format::Unknown;
}

auto get_best_standard_format(std::uint16_t type_flags,
                              std::uint32_t search_flags,
                              bool requires_alpha,
                              bool accept_padding) -> texture_format
{
    if(search_flags & format_search_flags::four_channels)
    {
        if(requires_alpha)
        {
            if(is_format_supported(type_flags, texture_format::BGRA8))
                return texture_format::BGRA8;
            if(is_format_supported(type_flags, texture_format::RGBA8))
                return texture_format::RGBA8;
            if(is_format_supported(type_flags, texture_format::RGBA16))
                return texture_format::RGBA16;
            if(is_format_supported(type_flags, texture_format::RGB10A2))
                return texture_format::RGB10A2;
            if(is_format_supported(type_flags, texture_format::RGB5A1))
                return texture_format::RGB5A1;
        }
        else
        {
            if(is_format_supported(type_flags, texture_format::BGRA8))
                return texture_format::BGRA8;
            if(is_format_supported(type_flags, texture_format::RGBA8))
                return texture_format::RGBA8;
            if(is_format_supported(type_flags, texture_format::RGB8))
                return texture_format::RGB8;
            if(is_format_supported(type_flags, texture_format::RGB10A2))
                return texture_format::RGB10A2;
            if(is_format_supported(type_flags, texture_format::RGBA16))
                return texture_format::RGBA16;
            if(is_format_supported(type_flags, texture_format::R5G6B5))
                return texture_format::R5G6B5;
            if(is_format_supported(type_flags, texture_format::RGB5A1))
                return texture_format::RGB5A1;
        }
    }
    else if(search_flags & format_search_flags::two_channels)
    {
        if(!requires_alpha)
        {
            if(is_format_supported(type_flags, texture_format::RG16))
                return texture_format::RG16;
            if(accept_padding)
            {
                if(is_format_supported(type_flags, texture_format::RGB8))
                    return texture_format::RGB8;
                if(is_format_supported(type_flags, texture_format::BGRA8))
                    return texture_format::BGRA8;
                if(is_format_supported(type_flags, texture_format::RGBA8))
                    return texture_format::RGBA8;
                if(is_format_supported(type_flags, texture_format::RGB10A2))
                    return texture_format::RGB10A2;
                if(is_format_supported(type_flags, texture_format::RGBA16))
                    return texture_format::RGBA16;
                if(is_format_supported(type_flags, texture_format::R5G6B5))
                    return texture_format::R5G6B5;
                if(is_format_supported(type_flags, texture_format::RGB5A1))
                    return texture_format::RGB5A1;
            }
        }
        else
        {
            if(accept_padding)
            {
                if(is_format_supported(type_flags, texture_format::BGRA8))
                    return texture_format::BGRA8;
                if(is_format_supported(type_flags, texture_format::RGBA8))
                    return texture_format::RGBA8;
                if(is_format_supported(type_flags, texture_format::RGBA16))
                    return texture_format::RGBA16;
                if(is_format_supported(type_flags, texture_format::RGB10A2))
                    return texture_format::RGB10A2;
                if(is_format_supported(type_flags, texture_format::RGB5A1))
                    return texture_format::RGB5A1;
            }
        }
    }
    else if(search_flags & format_search_flags::one_channel)
    {
        if(!requires_alpha)
        {
            if(is_format_supported(type_flags, texture_format::R8))
                return texture_format::R8;
            if(accept_padding)
            {
                if(is_format_supported(type_flags, texture_format::RG16))
                    return texture_format::RG16;
                if(is_format_supported(type_flags, texture_format::RGB8))
                    return texture_format::RGB8;
                if(is_format_supported(type_flags, texture_format::BGRA8))
                    return texture_format::BGRA8;
                if(is_format_supported(type_flags, texture_format::RGBA8))
                    return texture_format::RGBA8;
                if(is_format_supported(type_flags, texture_format::RGB10A2))
                    return texture_format::RGB10A2;
                if(is_format_supported(type_flags, texture_format::RGBA16))
                    return texture_format::RGBA16;
                if(is_format_supported(type_flags, texture_format::R5G6B5))
                    return texture_format::R5G6B5;
                if(is_format_supported(type_flags, texture_format::RGB5A1))
                    return texture_format::RGB5A1;
            }
        }
        else
        {
            if(is_format_supported(type_flags, texture_format::A8))
                return texture_format::A8;
            if(accept_padding)
            {
                if(is_format_supported(type_flags, texture_format::BGRA8))
                    return texture_format::BGRA8;
                if(is_format_supported(type_flags, texture_format::RGBA8))
                    return texture_format::RGBA8;
                if(is_format_supported(type_flags, texture_format::RGBA16))
                    return texture_format::RGBA16;
                if(is_format_supported(type_flags, texture_format::RGB10A2))
                    return texture_format::RGB10A2;
                if(is_format_supported(type_flags, texture_format::RGB5A1))
                    return texture_format::RGB5A1;
            }
        }
    }

    return texture_format::Unknown;
}

auto get_best_depth_format(std::uint16_t type_flags, std::uint32_t search_flags) -> texture_format
{
    bool requires_stencil = (search_flags & format_search_flags::requires_stencil) != 0;
    bool accept_full = (search_flags & format_search_flags::full_precision_float) != 0;

    if(search_flags & format_search_flags::floating_point)
    {
        if(!requires_stencil)
        {
            if(accept_full && is_format_supported(type_flags, texture_format::D32F))
                return texture_format::D32F;
            if(accept_full && is_format_supported(type_flags, texture_format::D24F))
                return texture_format::D24F;
        }
    }
    else
    {
        if(!requires_stencil)
        {
            if(is_format_supported(type_flags, texture_format::D32))
                return texture_format::D32;
            if(is_format_supported(type_flags, texture_format::D24))
                return texture_format::D24;
            if(is_format_supported(type_flags, texture_format::D16))
                return texture_format::D16;
        }
        else
        {
            if(is_format_supported(type_flags, texture_format::D24S8))
                return texture_format::D24S8;
        }
    }

    return texture_format::Unknown;
}

auto get_best_format(std::uint16_t type_flags, std::uint32_t search_flags) -> texture_format
{
    bool is_depth = (search_flags & format_search_flags::requires_depth) != 0;
    bool requires_alpha = (search_flags & format_search_flags::requires_alpha) != 0;
    bool accept_padding = (search_flags & format_search_flags::allow_padding_channels) != 0;
    bool accept_half = (search_flags & format_search_flags::half_precision_float) != 0;
    bool accept_full = (search_flags & format_search_flags::full_precision_float) != 0;

    if(!is_depth)
    {
        if((search_flags & format_search_flags::prefer_compressed) &&
           (search_flags & format_search_flags::four_channels) && !(search_flags & format_search_flags::floating_point))
        {
            if(requires_alpha)
            {
                if(is_format_supported(type_flags, texture_format::BC2))
                    return texture_format::BC2;
                if(is_format_supported(type_flags, texture_format::BC3))
                    return texture_format::BC3;
            }
            else
            {
                if(is_format_supported(type_flags, texture_format::BC1))
                    return texture_format::BC1;
            }
        }

        if(search_flags & format_search_flags::floating_point)
        {
            return get_best_float_format(type_flags,
                                         search_flags,
                                         requires_alpha,
                                         accept_padding,
                                         accept_half,
                                         accept_full);
        }
        else
        {
            return get_best_standard_format(type_flags, search_flags, requires_alpha, accept_padding);
        }
    }
    else
    {
        return get_best_depth_format(type_flags, search_flags);
    }

    return texture_format::Unknown;
}

auto get_default_rt_sampler_flags() -> uint64_t
{
    static std::uint64_t sampler_flags = 0 | BGFX_TEXTURE_RT | BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP;

    return sampler_flags;
}

auto is_format_supported(uint16_t flags, texture_format format) -> bool
{
    const std::uint32_t formatCaps = bgfx::getCaps()->formats[format];
    return 0 != (formatCaps & flags);
}
} // namespace gfx
