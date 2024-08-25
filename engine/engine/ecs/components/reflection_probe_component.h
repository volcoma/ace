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
                                       const math::vec3& scale,
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
    auto get_cubemap() -> const gfx::texture::ptr&;

    /**
     * @brief Gets the cubemap frame buffer object (FBO).
     * @return A shared pointer to the cubemap frame buffer object.
     */
    auto get_cubemap_fbo(size_t face) -> const gfx::frame_buffer::ptr&;

    /**
     * @brief Updates the reflection probe component.
     */
    void update();

    /**
     * @brief Check if the cubemap was generated this frame.
     * @return A bool indicating whether the faces was already generated
     */
    auto already_generated() const -> bool;

    /**
     * @brief Check if the cubemap's face was generated this frame.
     * @return A bool indicating whether the face was already generated.
     */
    auto already_generated(size_t face) const -> bool;

    /**
     * @brief marks the genrerated face this frame
     */
    void set_generation_frame(size_t face, uint64_t frame);

private:

    /**
     * @brief The reflection probe object this component represents.
     */
    reflection_probe probe_;

    /**
     * @brief The render views for this component.
     */
    std::array<gfx::render_view, 6> rview_;

    std::array<uint64_t, 6> generated_frame_{uint64_t(-1),
                                             uint64_t(-1),
                                             uint64_t(-1),
                                             uint64_t(-1),
                                             uint64_t(-1),
                                             uint64_t(-1)};

    size_t faces_per_frame_ = 1;       // Number of faces to generate per frame
    size_t generated_faces_count_ = 0; // Number of faces generated in the current cycle
    bool first_generation_{true};
};

} // namespace ace
