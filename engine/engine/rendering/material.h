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

/**
 * @enum cull_type
 * @brief Enum representing the type of culling to be used.
 */
enum class cull_type : std::uint32_t
{
    none,              ///< No culling.
    clockwise,         ///< Cull clockwise faces.
    counter_clockwise, ///< Cull counter-clockwise faces.
};

/**
 * @class material
 * @brief Base class for materials used in rendering.
 */
class material
{
public:
    SERIALIZABLE(material)
    REFLECTABLE(material)

    using sptr = std::shared_ptr<material>;
    using wptr = std::weak_ptr<material>;
    using uptr = std::unique_ptr<material>;

    material() = default;
    virtual ~material() = default;

    /**
     * @brief Submits the material properties to the GPU program.
     * @param program The GPU program to submit the properties to.
     */
    virtual void submit(gpu_program* program) const
    {
    }

    /**
     * @brief Gets the culling type of the material.
     * @return The culling type.
     */
    inline auto get_cull_type() const -> cull_type
    {
        return cull_type_;
    }

    /**
     * @brief Sets the culling type of the material.
     * @param val The culling type to set.
     */
    inline void set_cull_type(cull_type val)
    {
        cull_type_ = val;
    }

    /**
     * @brief Gets the render states for the material.
     * @param apply_cull Whether to apply culling.
     * @param depth_write Whether to write to the depth buffer.
     * @param depth_test Whether to perform depth testing.
     * @return The render states as a 64-bit integer.
     */
    virtual auto get_render_states(bool apply_cull = true, bool depth_write = true, bool depth_test = true) const
        -> uint64_t;

    /**
     * @brief Gets the default color map.
     * @return A reference to the default color map asset handle.
     */
    static auto default_color_map() -> asset_handle<gfx::texture>&;

    /**
     * @brief Gets the default normal map.
     * @return A reference to the default normal map asset handle.
     */
    static auto default_normal_map() -> asset_handle<gfx::texture>&;

protected:
    cull_type cull_type_ = cull_type::counter_clockwise; ///< The culling type for this material.
};

/**
 * @class pbr_material
 * @brief Class for physically-based rendering (PBR) materials.
 */
class pbr_material : public material
{
public:
    SERIALIZABLE(pbr_material)
    REFLECTABLEV(pbr_material, material)

    /**
     * @brief Gets the base color of the material.
     * @return A constant reference to the base color.
     */
    inline auto get_base_color() const -> const math::color&
    {
        return base_color_;
    }

    /**
     * @brief Sets the base color of the material.
     * @param val The base color to set.
     */
    inline void set_base_color(const math::color& val)
    {
        base_color_ = val;
    }

    /**
     * @brief Gets the subsurface color of the material.
     * @return A constant reference to the subsurface color.
     */
    inline auto get_subsurface_color() const -> const math::color&
    {
        return subsurface_color_;
    }

    /**
     * @brief Sets the subsurface color of the material.
     * @param val The subsurface color to set.
     */
    inline void set_subsurface_color(const math::color& val)
    {
        subsurface_color_ = val;
    }

    /**
     * @brief Gets the emissive color of the material.
     * @return A constant reference to the emissive color.
     */
    inline auto get_emissive_color() const -> const math::color&
    {
        return emissive_color_;
    }

    /**
     * @brief Sets the emissive color of the material.
     * @param val The emissive color to set.
     */
    inline void set_emissive_color(const math::color& val)
    {
        emissive_color_ = val;
    }

    /**
     * @brief Gets the roughness of the material.
     * @return The roughness value.
     */
    inline auto get_roughness() const -> float
    {
        return surface_data_.x;
    }

    /**
     * @brief Sets the roughness of the material.
     * @param roughness The roughness value to set.
     */
    inline void set_roughness(float roughness)
    {
        surface_data_.x = roughness;
    }

    /**
     * @brief Gets the metalness of the material.
     * @return The metalness value.
     */
    inline auto get_metalness() const -> float
    {
        return surface_data_.y;
    }

    /**
     * @brief Sets the metalness of the material.
     * @param metalness The metalness value to set.
     */
    inline void set_metalness(float metalness)
    {
        surface_data_.y = metalness;
    }

    /**
     * @brief Gets the bumpiness of the material.
     * @return The bumpiness value.
     */
    inline auto get_bumpiness() const -> float
    {
        return surface_data_.z;
    }

    /**
     * @brief Sets the bumpiness of the material.
     * @param bumpiness The bumpiness value to set.
     */
    inline void set_bumpiness(float bumpiness)
    {
        surface_data_.z = bumpiness;
    }

    /**
     * @brief Gets the alpha test value of the material.
     * @return The alpha test value.
     */
    inline auto get_alpha_test_value() const -> float
    {
        return surface_data_.w;
    }

    /**
     * @brief Sets the alpha test value of the material.
     * @param alphaTestValue The alpha test value to set.
     */
    inline void set_alpha_test_value(float alphaTestValue)
    {
        surface_data_.w = alphaTestValue;
    }

    /**
     * @brief Gets the surface data of the material.
     * @return A constant reference to the surface data vector.
     */
    inline auto get_surface_data() const -> const math::vec4&
    {
        return surface_data_;
    }

    /**
     * @brief Gets additional surface data for the material.
     * @return A vector containing additional surface data.
     */
    inline auto get_surface_data2() const -> math::vec4
    {
        math::vec4 surface_data2{};

        surface_data2[0] = metalness_roughness_combined() ? 1.0f : 0.0f;

        return surface_data2;
    }

    inline auto metalness_roughness_combined() const -> bool
    {
        return metalness_map_ == roughness_map_;
    }

    /**
     * @brief Gets the tiling factor of the material.
     * @return A constant reference to the tiling vector.
     */
    inline auto get_tiling() const -> const math::vec2&
    {
        return tiling_;
    }

    /**
     * @brief Sets the tiling factor of the material.
     * @param tiling The tiling factor to set.
     */
    inline void set_tiling(const math::vec2& tiling)
    {
        tiling_ = tiling;
    }

    /**
     * @brief Gets the dither threshold of the material.
     * @return A constant reference to the dither threshold vector.
     */
    inline auto get_dither_threshold() const -> const math::vec2&
    {
        return dither_threshold_;
    }

    /**
     * @brief Sets the dither threshold of the material.
     * @param threshold The dither threshold to set.
     */
    inline void set_dither_threshold(const math::vec2& threshold)
    {
        dither_threshold_ = threshold;
    }

    /**
     * @brief Gets the color map of the material.
     * @return A constant reference to the color map asset handle.
     */
    inline auto get_color_map() const -> const asset_handle<gfx::texture>&
    {
        return color_map_;
    }

    /**
     * @brief Sets the color map of the material.
     * @param val The color map asset handle to set.
     */
    inline void set_color_map(const asset_handle<gfx::texture>& val)
    {
        color_map_ = val;
    }

    /**
     * @brief Gets the normal map of the material.
     * @return A constant reference to the normal map asset handle.
     */
    inline auto get_normal_map() const -> const asset_handle<gfx::texture>&
    {
        return normal_map_;
    }

    /**
     * @brief Sets the normal map of the material.
     * @param val The normal map asset handle to set.
     */
    inline void set_normal_map(const asset_handle<gfx::texture>& val)
    {
        normal_map_ = val;
    }

    /**
     * @brief Gets the roughness map of the material.
     * @return A constant reference to the roughness map asset handle.
     */
    inline auto get_roughness_map() const -> const asset_handle<gfx::texture>&
    {
        return roughness_map_;
    }

    /**
     * @brief Sets the roughness map of the material.
     * @param val The roughness map asset handle to set.
     */
    inline void set_roughness_map(const asset_handle<gfx::texture>& val)
    {
        roughness_map_ = val;
    }

    /**
     * @brief Gets the metalness map of the material.
     * @return A constant reference to the metalness map asset handle.
     */
    inline auto get_metalness_map() const -> const asset_handle<gfx::texture>&
    {
        return metalness_map_;
    }

    /**
     * @brief Sets the metalness map of the material.
     * @param val The metalness map asset handle to set.
     */
    inline void set_metalness_map(const asset_handle<gfx::texture>& val)
    {
        metalness_map_ = val;
    }

    /**
     * @brief Gets the ambient occlusion map of the material.
     * @return A constant reference to the ambient occlusion map asset handle.
     */
    inline auto get_ao_map() const -> const asset_handle<gfx::texture>&
    {
        return ao_map_;
    }

    /**
     * @brief Sets the ambient occlusion map of the material.
     * @param val The ambient occlusion map asset handle to set.
     */
    inline void set_ao_map(const asset_handle<gfx::texture>& val)
    {
        ao_map_ = val;
    }

    /**
     * @brief Gets the emissive map of the material.
     * @return A constant reference to the emissive map asset handle.
     */
    inline auto get_emissive_map() const -> const asset_handle<gfx::texture>&
    {
        return emissive_map_;
    }

    /**
     * @brief Sets the emissive map of the material.
     * @param val The emissive map asset handle to set.
     */
    inline void set_emissive_map(const asset_handle<gfx::texture>& val)
    {
        emissive_map_ = val;
    }

private:
    /// Base color
    math::color base_color_{
        1.0f,
        1.0f,
        1.0f, /// Color
        1.0f  /// Opacity
    };
    /// Subsurface color
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
    asset_handle<gfx::texture> color_map_;
    asset_handle<gfx::texture> normal_map_;
    asset_handle<gfx::texture> roughness_map_;
    asset_handle<gfx::texture> metalness_map_;
    asset_handle<gfx::texture> emissive_map_;
    asset_handle<gfx::texture> ao_map_;
};

} // namespace ace
