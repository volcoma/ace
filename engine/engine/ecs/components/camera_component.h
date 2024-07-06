#pragma once
#include "basic_component.h"
#include <base/basetypes.hpp>
#include <engine/rendering/camera.h>
#include <graphics/render_pass.h>
#include <graphics/render_view.h>
#include <math/math.h>

namespace ace
{
/**
 * @class camera_component
 * @brief Class that contains core camera data, used for rendering and other purposes.
 */
class camera_component : public component_crtp<camera_component>
{
public:
    /**
     * @brief Default constructor for camera_component.
     */
    camera_component();

    /**
     * @brief Sets the field of view (FOV).
     * @param[in] fovDegrees The FOV in degrees.
     */
    void set_fov(float fovDegrees);

    /**
     * @brief Sets the near clipping distance.
     * @param[in] distance The near clipping distance.
     */
    void set_near_clip(float distance);

    /**
     * @brief Sets the far clipping distance.
     * @param[in] distance The far clipping distance.
     */
    void set_far_clip(float distance);

    /**
     * @brief Sets the projection mode.
     * @param[in] mode The projection mode to set.
     */
    void set_projection_mode(projection_mode mode);

    /**
     * @brief Gets the field of view (FOV).
     * @return The FOV in degrees.
     */
    auto get_fov() const -> float;

    /**
     * @brief Gets the near clipping distance.
     * @return The near clipping distance.
     */
    auto get_near_clip() const -> float;

    /**
     * @brief Gets the far clipping distance.
     * @return The far clipping distance.
     */
    auto get_far_clip() const -> float;

    /**
     * @brief Gets the projection mode.
     * @return The current projection mode.
     */
    auto get_projection_mode() const -> projection_mode;

    /**
     * @brief Gets the camera object.
     * @return A reference to the camera object.
     */
    auto get_camera() -> camera&;

    /**
     * @brief Gets the camera object (const version).
     * @return A constant reference to the camera object.
     */
    auto get_camera() const -> const camera&;

    /**
     * @brief Updates the camera with the given transform.
     * @param[in] t The transform to update the camera with.
     */
    void update(const math::transform& t);

    /**
     * @brief Gets the HDR status of the camera.
     * @return True if HDR is enabled, otherwise false.
     */
    auto get_hdr() const -> bool;

    /**
     * @brief Sets the HDR status of the camera.
     * @param[in] hdr The HDR status to set.
     */
    void set_hdr(bool hdr);

    /**
     * @brief Sets the viewport size.
     * @param[in] size The size of the viewport.
     */
    void set_viewport_size(const usize32_t& size);

    /**
     * @brief Gets the viewport size.
     * @return A constant reference to the size of the viewport.
     */
    auto get_viewport_size() const -> const usize32_t&;

    /**
     * @brief Gets the orthographic size.
     * @return The orthographic size.
     */
    auto get_ortho_size() const -> float;

    /**
     * @brief Sets the orthographic size.
     * @param[in] size The orthographic size to set.
     */
    void set_ortho_size(float size);

    /**
     * @brief Gets the pixels per unit (PPU).
     * @return The pixels per unit.
     */
    auto get_ppu() const -> float;

    /**
     * @brief Gets the render view.
     * @return A reference to the render view.
     */
    auto get_render_view() -> gfx::render_view&;

    /**
     * @brief Gets the camera storage.
     * @return A reference to the camera storage.
     */
    auto get_storage() -> camera_storage&;

private:
    /// @brief The camera object this component represents
    camera camera_;

    /// @brief The render view for this component
    gfx::render_view render_view_;

    /// @brief Is the camera HDR?
    bool hdr_ = true;

    /// @brief The camera storage
    camera_storage storage_;
};

} // namespace ace
