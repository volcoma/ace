#pragma once
#include "basic_component.h"
#include <engine/rendering/reflection_probe.h>

#include <array>
#include <base/basetypes.hpp>
#include <graphics/render_pass.h>
#include <graphics/render_view.h>

namespace ace
{

/**
 * @class reflection_probe_component
 * @brief Class that contains core reflection probe data, used for rendering and other purposes.
 */
class reflection_probe_component : public component_crtp<reflection_probe_component>
{
public:
    /**
     * @brief Gets the reflection probe object.
     * @return A constant reference to the reflection probe object.
     */
    auto get_probe() const -> const reflection_probe&;

    /**
     * @brief Sets the reflection probe object.
     * @param[in] probe The reflection probe object to set.
     */
    void set_probe(const reflection_probe& probe);

    /**
     * @brief Gets the bounding box of the probe object.
     */
    auto get_bounds() const -> math::bbox;

    /**
     * @brief Computes the projected sphere rectangle.
     * @param[out] rect Reference to the rectangle to be computed.
     * @param[in] position The position of the reflection probe.
     * @param[in] view_origin The origin of the view.
     * @param[in] view The view transform.
     * @param[in] proj The projection transform.
     * @return An integer indicating the result of the computation.
     */
    auto compute_projected_sphere_rect(irect32_t& rect,
                                       const math::vec3& position,
                                       const math::vec3& view_origin,
                                       const math::transform& view,
                                       const math::transform& proj) const -> int;

    /**
     * @brief Gets the render view.
     * @param[in] idx The index of the render view to get.
     * @return A reference to the render view.
     */
    auto get_render_view(size_t idx) -> gfx::render_view&;

    /**
     * @brief Gets the cubemap texture.
     * @return A shared pointer to the cubemap texture.
     */
    auto get_cubemap() -> std::shared_ptr<gfx::texture>;

    /**
     * @brief Gets the cubemap frame buffer object (FBO).
     * @return A shared pointer to the cubemap frame buffer object.
     */
    auto get_cubemap_fbo() -> std::shared_ptr<gfx::frame_buffer>;

    /**
     * @brief Updates the reflection probe component.
     */
    void update();

    auto already_generated() const -> bool;
    void set_generation_frame(uint64_t frame);
private:

    /**
     * @brief Releases resources associated with the reflection probe component.
     */
    void release_resources();

    /**
     * @brief The reflection probe object this component represents.
     */
    reflection_probe probe_;

    /**
     * @brief The render views for this component.
     */
    std::array<gfx::render_view, 6> render_view_;

    uint64_t generated_frame_ = -1;
};

} // namespace ace
