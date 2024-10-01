#pragma once
#include <base/basetypes.hpp>
#include <engine/ecs/components/basic_component.h>
#include <engine/rendering/light.h>
#include <engine/rendering/shadow.h>

namespace ace
{

/**
 * @class light_component
 * @brief Class that contains core light data, used for rendering and other purposes.
 */
class light_component : public component_crtp<light_component>
{
public:
    /**
     * @brief Gets the light object.
     * @return A constant reference to the light object.
     */
    auto get_light() const -> const light&;

    /**
     * @brief Sets the light object.
     * @param[in] l The light object to set.
     */
    void set_light(const light& l);

    /**
     * @brief Gets the bounding box of the light object.
     */
    auto get_bounds() const -> math::bbox;
    auto get_bounds_sphere() const -> math::bsphere;

    /**
     * @brief Gets the bounding box of the light object.
     */
    auto get_bounds_precise(const math::vec3& light_direction) const -> math::bbox;
    auto get_bounds_sphere_precise(const math::vec3& light_direction) const -> math::bsphere;

    /**
     * @brief Computes the projected sphere rectangle.
     * @param[out] rect Reference to the rectangle to be computed.
     * @param[in] light_position The position of the light.
     * @param[in] light_direction The direction of the light.
     * @param[in] view_origin The origin of the view.
     * @param[in] view The view transform.
     * @param[in] proj The projection transform.
     * @return An integer indicating the result of the computation.
     */
    auto compute_projected_sphere_rect(irect32_t& rect,
                                       const math::vec3& light_position,
                                       const math::vec3& light_direction,
                                       const math::vec3& view_origin,
                                       const math::transform& view,
                                       const math::transform& proj) -> int;

    /**
     * @brief Gets the shadow map generator.
     * @return A reference to the shadow map generator.
     */
    auto get_shadowmap_generator() -> shadow::shadowmap_generator&;

private:
    auto get_bounds_sphere_impl(const math::vec3* light_direction) const -> math::bsphere;

    /**
     * @brief The light object this component represents.
     */
    light light_;

    /**
     * @brief The shadow map generator.
     */
    shadow::shadowmap_generator shadowmap_generator_;
};

/**
 * @class skylight_component
 * @brief Class that contains sky light data.
 */
class skylight_component : public component_crtp<skylight_component>
{
public:
    /**
     * @enum sky_mode
     * @brief Enumeration for sky modes.
     */
    enum class sky_mode
    {
        /// Standard sky mode
        standard,
        /// Perez sky mode
        perez
    };

    /**
     * @brief Gets the current sky mode.
     * @return A constant reference to the current sky mode.
     */
    auto get_mode() const noexcept -> const sky_mode&;

    /**
     * @brief Sets the sky mode.
     * @param[in] mode The sky mode to set.
     */
    void set_mode(const sky_mode& mode);

    /**
     * @brief Gets the current turbidity value.
     * @return The current turbidity value.
     */
    auto get_turbidity() const noexcept -> float;

    /**
     * @brief Sets the turbidity value.
     * @param[in] turbidity The turbidity value to set, in the range 1.9f-10.0f.
     */
    void set_turbidity(float turbidity);

private:
    /**
     * @brief The current sky mode.
     */
    sky_mode mode_{sky_mode::perez};

    /**
     * @brief The current turbidity value.
     */
    float turbidity_{1.9};
};

} // namespace ace
