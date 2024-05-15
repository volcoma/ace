#pragma once

#include "gpu_program.h"
#include <engine/assets/asset_handle.h>

#include <graphics/graphics.h>
#include <math/math.h>
#include <reflection/registration.h>
#include <serialization/serialization.h>

#include <unordered_map>

namespace ace
{

enum class cull_type : std::uint32_t
{
    none,
    clockwise,
    counter_clockwise,
};

class material
{
public:
    SERIALIZABLE(material)
    REFLECTABLE(material)

public:
    material() = default;
    virtual ~material() = default;

    virtual void submit(gpu_program* program) const
    {
    }

    inline cull_type get_cull_type() const
    {
        return cull_type_;
    }

    inline void set_cull_type(cull_type val)
    {
        cull_type_ = val;
    }

    virtual auto get_render_states(bool apply_cull = true, bool depth_write = true, bool depth_test = true) const
        -> std::uint64_t;

    /// Default color texture
    static asset_handle<gfx::texture>& default_color_map();
    /// Default normal texture
    static asset_handle<gfx::texture>& default_normal_map();

protected:
    /// Cull type for this material.
    cull_type cull_type_ = cull_type::counter_clockwise;
};

class pbr_material : public material
{
public:
    SERIALIZABLE(pbr_material)
    REFLECTABLEV(pbr_material, material)

    inline auto get_base_color() const -> const math::color&
    {
        return base_color_;
    }

    inline void set_base_color(const math::color& val)
    {
        base_color_ = val;
    }

    inline auto get_subsurface_color() const -> const math::color&
    {
        return subsurface_color_;
    }

    inline void set_subsurface_color(const math::color& val)
    {
        subsurface_color_ = val;
    }

    inline auto get_emissive_color() const -> const math::color&
    {
        return emissive_color_;
    }

    inline void set_emissive_color(const math::color& val)
    {
        emissive_color_ = val;
    }

    inline auto get_roughness() const -> float
    {
        return surface_data_.x;
    }

    inline void set_roughness(float rougness)
    {
        surface_data_.x = rougness;
    }

    inline auto get_metalness() const -> float
    {
        return surface_data_.y;
    }

    inline void set_metalness(float metalness)
    {
        surface_data_.y = metalness;
    }

    inline auto get_bumpiness() const -> float
    {
        return surface_data_.z;
    }

    inline void set_bumpiness(float bumpiness)
    {
        surface_data_.z = bumpiness;
    }

    inline auto get_alpha_test_value() const -> float
    {
        return surface_data_.w;
    }

    inline void set_alpha_test_value(float alphaTestValue)
    {
        surface_data_.w = alphaTestValue;
    }

    inline auto get_tiling() const -> const math::vec2&
    {
        return tiling_;
    }

    inline void set_tiling(const math::vec2& tiling)
    {
        tiling_ = tiling;
    }

    inline auto get_dither_threshold() const -> const math::vec2&
    {
        return dither_threshold_;
    }

    inline void set_dither_threshold(const math::vec2& threshold)
    {
        dither_threshold_ = threshold;
    }

    inline auto get_color_map() const -> const asset_handle<gfx::texture>&
    {
        return get_map("color");
    }

    inline void set_color_map(const asset_handle<gfx::texture>& val)
    {
        maps_["color"] = val;
    }

    inline auto get_normal_map() const -> const asset_handle<gfx::texture>&
    {
        return get_map("normal");
    }

    inline void set_normal_map(const asset_handle<gfx::texture>& val)
    {
        get_map("normal") = val;
    }

    inline auto get_roughness_map() const -> const asset_handle<gfx::texture>&
    {
        return get_map("roughness");
    }

    inline void set_roughness_map(const asset_handle<gfx::texture>& val)
    {
        get_map("roughness") = val;
    }

    inline auto get_metalness_map() const -> const asset_handle<gfx::texture>&
    {
        return get_map("metalness");
    }

    inline void set_metalness_map(const asset_handle<gfx::texture>& val)
    {
        get_map("metalness") = val;
    }

    inline auto get_ao_map() const -> const asset_handle<gfx::texture>&
    {
        return get_map("ao");
    }

    inline void set_ao_map(const asset_handle<gfx::texture>& val)
    {
        get_map("ao") = val;
    }

    inline auto get_emissive_map() const -> const asset_handle<gfx::texture>&
    {
        return get_map("emissive");
    }

    inline void set_emissive_map(const asset_handle<gfx::texture>& val)
    {
        get_map("emissive") = val;
    }

    virtual void submit(gpu_program* program) const override;

private:
    auto get_map(const hpp::string_view& id) const -> const asset_handle<gfx::texture>&;
    auto get_map(const hpp::string_view& id) -> asset_handle<gfx::texture>&;

    /// Base color
    math::color base_color_{
        1.0f,
        1.0f,
        1.0f, /// Color
        1.0f  /// Opacity
    };
    /// Emissive color
    math::color subsurface_color_{
        0.0f,
        0.0f,
        0.0f, /// Color
        0.8f  /// Opacity
    };
    /// Emissive color
    math::color emissive_color_{
        0.0f,
        0.0f,
        0.0f, /// Color
        0.0f  /// HDR Scale
    };
    /// Surface data
    math::vec4 surface_data_{
        0.3f, /// Roughness
        0.0f, /// Metalness
        1.0f, /// Bumpiness
        0.25f /// AlphaTestValue
    };
    /// Tiling data
    math::vec2 tiling_{
        1.0f,
        1.0f /// Primary
    };
    /// Dithering data
    math::vec2 dither_threshold_{
        0.5f, /// Alpha threshold
        0.0f  /// Distance threshold
    };
    /// Texture maps
    using textures_container = std::map<std::string, asset_handle<gfx::texture>, std::less<>>;
    textures_container maps_;
};

} // namespace ace
