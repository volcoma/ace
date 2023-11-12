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
    REFLECTABLEV(material)
    SERIALIZABLE(material)

    virtual ~material() = default;

    inline bool is_valid() const
    {
        return get_program().is_valid();
    }

    void set_texture(std::uint8_t _stage,
                     const std::string& _sampler,
                     gfx::frame_buffer* _handle,
                     uint8_t _attachment = 0,
                     std::uint32_t _flags = std::numeric_limits<std::uint32_t>::max());

    void set_texture(std::uint8_t _stage,
                     const std::string& _sampler,
                     gfx::texture* _texture,
                     std::uint32_t _flags = std::numeric_limits<std::uint32_t>::max());

    void set_uniform(const std::string& _name, const void* _value, std::uint16_t _num = 1);

    virtual gpu_program& get_program()
    {
        static gpu_program program;
        return program;
    }
    virtual const gpu_program& get_program() const
    {
        static gpu_program program;
        return program;
    }

    gpu_program* get_program_ptr()
    {
        return &get_program();
    }
    const gpu_program* get_program_ptr() const
    {
        return &get_program();
    }

    virtual void submit()
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

    std::uint64_t get_render_states(bool apply_cull = true, bool depth_write = true, bool depth_test = true) const;

    /// Default color texture
    static asset_handle<gfx::texture>& default_color_map();
    /// Default normal texture
    static asset_handle<gfx::texture>& default_normal_map();

protected:
    /// Cull type for this material.
    cull_type cull_type_ = cull_type::counter_clockwise;
};

class standard_material : public material
{
public:
    SERIALIZABLE(standard_material)
    REFLECTABLEV(standard_material, material)

    gpu_program& get_program() override;
    const gpu_program& get_program() const override;

    inline const math::color& get_base_color() const
    {
        return base_color_;
    }

    inline void set_base_color(const math::color& val)
    {
        base_color_ = val;
    }

    inline const math::color& get_subsurface_color() const
    {
        return subsurface_color_;
    }

    inline void set_subsurface_color(const math::color& val)
    {
        subsurface_color_ = val;
    }

    inline const math::color& get_emissive_color() const
    {
        return emissive_color_;
    }

    inline void set_emissive_color(const math::color& val)
    {
        emissive_color_ = val;
    }

    inline float get_roughness() const
    {
        return surface_data_.x;
    }

    inline void set_roughness(float rougness)
    {
        surface_data_.x = rougness;
    }

    inline float get_metalness() const
    {
        return surface_data_.y;
    }

    inline void set_metalness(float metalness)
    {
        surface_data_.y = metalness;
    }

    inline float get_bumpiness() const
    {
        return surface_data_.z;
    }

    inline void set_bumpiness(float bumpiness)
    {
        surface_data_.z = bumpiness;
    }

    inline float get_alpha_test_value() const
    {
        return surface_data_.w;
    }

    inline void set_alpha_test_value(float alphaTestValue)
    {
        surface_data_.w = alphaTestValue;
    }

    inline const math::vec2& get_tiling() const
    {
        return tiling_;
    }

    inline void set_tiling(const math::vec2& tiling)
    {
        tiling_ = tiling;
    }

    inline const math::vec2& get_dither_threshold() const
    {
        return dither_threshold_;
    }

    inline void set_dither_threshold(const math::vec2& threshold)
    {
        dither_threshold_ = threshold;
    }

    inline asset_handle<gfx::texture> get_color_map()
    {
        return maps_["color"];
    }

    inline void set_color_map(asset_handle<gfx::texture> val)
    {
        maps_["color"] = val;
    }

    inline asset_handle<gfx::texture> get_normal_map()
    {
        return maps_["normal"];
    }

    inline void set_normal_map(asset_handle<gfx::texture> val)
    {
        maps_["normal"] = val;
    }

    inline asset_handle<gfx::texture> get_roughness_map()
    {
        return maps_["roughness"];
    }

    inline void set_roughness_map(asset_handle<gfx::texture> val)
    {
        maps_["roughness"] = val;
    }

    inline asset_handle<gfx::texture> get_metalness_map()
    {
        return maps_["metalness"];
    }

    inline void set_metalness_map(asset_handle<gfx::texture> val)
    {
        maps_["metalness"] = val;
    }

    inline asset_handle<gfx::texture> get_ao_map()
    {
        return maps_["ao"];
    }

    inline void set_ao_map(asset_handle<gfx::texture> val)
    {
        maps_["ao"] = val;
    }

    virtual void submit();

    bool skinned = false;

    /// Program that is responsible for rendering.
    static gpu_program& program();
    /// Program that is responsible for rendering.
    static gpu_program& program_skinned();

private:
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
    std::unordered_map<std::string, asset_handle<gfx::texture>> maps_;
};

} // namespace ace
