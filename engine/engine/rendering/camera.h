#pragma once
#include <engine/engine_export.h>

#include <base/basetypes.hpp>
#include <context/context.hpp>
#include <math/math.h>
#include <reflection/registration.h>
#include <serialization/serialization.h>

namespace ace
{
/**
 * @brief Enum representing the projection mode of a camera.
 */
enum class projection_mode : std::uint32_t
{
    perspective = 0,
    orthographic = 1
};

/**
 * @brief Structure for storing camera related context.
 */
struct camera_storage
{
    rtti::context ctx; ///< RTTI context for the camera.
};

/**
 * @brief Class representing a camera. Contains functionality for manipulating and
 * updating a camera. It should not be used as a standalone class - see
 * camera_component and the entity system.
 */
class camera
{
public:
    REFLECTABLE(camera)
    SERIALIZABLE(camera)

    /**
     * @brief Sets the current projection mode for this camera (i.e. orthographic or perspective).
     *
     * @param mode The projection mode to set.
     */
    void set_projection_mode(projection_mode mode);

    /**
     * @brief Sets the field of view angle of this camera (perspective only).
     *
     * @param degrees The field of view in degrees.
     */
    void set_fov(float degrees);

    /**
     * @brief Sets the near plane distance.
     *
     * @param distance The distance to the near clipping plane.
     */
    void set_near_clip(float distance);

    /**
     * @brief Sets the far plane distance.
     *
     * @param distance The distance to the far clipping plane.
     */
    void set_far_clip(float distance);

    /**
     * @brief Sets the half of the vertical size of the viewing volume in world units.
     *
     * @param size The size to set.
     */
    void set_orthographic_size(float size);

    /**
     * @brief Retrieves the current projection mode for this camera.
     *
     * @return The current projection mode.
     */
    auto get_projection_mode() const -> projection_mode;

    /**
     * @brief Retrieves the current field of view angle in degrees.
     *
     * @return The field of view angle.
     */
    auto get_fov() const -> float;

    /**
     * @brief Retrieves the distance from the camera to the near clip plane.
     *
     * @return The near clip distance.
     */
    auto get_near_clip() const -> float;

    /**
     * @brief Retrieves the distance from the camera to the far clip plane.
     *
     * @return The far clip distance.
     */
    auto get_far_clip() const -> float;

    /**
     * @brief Retrieves the orthographic size.
     *
     * @return The orthographic size.
     */
    auto get_ortho_size() const -> float;

    /**
     * @brief Retrieves the zoom factor.
     *
     * @return The zoom factor.
     */
    float get_zoom_factor() const;

    /**
     * @brief Retrieves the pixels per unit (PPU).
     *
     * @return The PPU value.
     */
    float get_ppu() const;

    /**
     * @brief Sets the size of the viewport.
     *
     * @param viewportSize The size of the viewport.
     */
    void set_viewport_size(const usize32_t& viewportSize);

    /**
     * @brief Sets the position of the viewport.
     *
     * @param viewportPos The position of the viewport.
     */
    void set_viewport_pos(const upoint32_t& viewportPos);

    /**
     * @brief Retrieves the size of the viewport.
     *
     * @return The size of the viewport.
     */
    auto get_viewport_size() const -> const usize32_t&;

    /**
     * @brief Retrieves the position of the viewport.
     *
     * @return The position of the viewport.
     */
    auto get_viewport_pos() const -> const upoint32_t&;

    /**
     * @brief Sets the aspect ratio to be used for generating the horizontal FOV angle (perspective only).
     *
     * @param aspect The aspect ratio to set.
     * @param locked Whether the aspect ratio should be locked.
     */
    void set_aspect_ratio(float aspect, bool locked = false);

    /**
     * @brief Retrieves the aspect ratio used to generate the horizontal FOV angle.
     *
     * @return The aspect ratio.
     */
    auto get_aspect_ratio() const -> float;

    /**
     * @brief Determines if the aspect ratio is currently being updated by the render driver.
     *
     * @return true if the aspect ratio is locked, false otherwise.
     */
    bool is_aspect_locked() const;

    /**
     * @brief Checks if the frustum is currently locked.
     *
     * @return true if the frustum is locked, false otherwise.
     */
    auto is_frustum_locked() const -> bool;

    /**
     * @brief Locks or unlocks the frustum.
     *
     * @param locked Whether the frustum should be locked.
     */
    void lock_frustum(bool locked);

    /**
     * @brief Retrieves the current camera object frustum.
     *
     * @return The current frustum.
     */
    auto get_frustum() const -> const math::frustum&;

    /**
     * @brief Retrieves the frustum representing the space between the camera position and its near plane.
     *
     * @return The clipping volume frustum.
     */
    auto get_clipping_volume() const -> const math::frustum&;

    /**
     * @brief Retrieves the current projection matrix.
     *
     * @return The current projection matrix.
     */
    auto get_projection() const -> const math::transform&;

    /**
     * @brief Retrieves the current view matrix.
     *
     * @return The current view matrix.
     */
    auto get_view() const -> const math::transform&;
    auto get_view_inverse() const -> const math::transform&;

    /**
     * @brief Retrieves the current view-projection matrix.
     *
     * @return The current view-projection matrix.
     */
    auto get_view_projection() const -> math::transform;

    /**
     * @brief Makes a copy of the current view and projection matrices before they are changed.
     */
    void record_current_matrices();

    /**
     * @brief Sets the current jitter value for temporal anti-aliasing.
     *
     * @param viewportSize The size of the viewport.
     * @param currentSubpixelIndex The current subpixel index.
     * @param temporalAASamples The number of temporal AA samples.
     */
    void set_aa_data(const usize32_t& viewportSize,
                     std::uint32_t currentSubpixelIndex,
                     std::uint32_t temporalAASamples);

    /**
     * @brief Retrieves the anti-aliasing data.
     *
     * @return The anti-aliasing data.
     */
    auto get_aa_data() const -> const math::vec4&;

    /**
     * @brief Determines if the specified AABB falls within the frustum.
     *
     * @param bounds The AABB to test.
     * @return The result of the volume query.
     */
    auto classify_aabb(const math::bbox& bounds) const -> math::volume_query;

    /**
     * @brief Tests if the specified AABB is within the frustum.
     *
     * @param bounds The AABB to test.
     * @return true if the AABB is within the frustum, false otherwise.
     */
    auto test_aabb(const math::bbox& bounds) const -> bool;

    /**
     * @brief Determines if the specified OBB is within the frustum.
     *
     * @param bounds The OBB to test.
     * @param t The transformation to apply to the OBB.
     * @return The result of the volume query.
     */
    auto classify_obb(const math::bbox& bounds, const math::transform& t) const -> math::volume_query;

    /**
     * @brief Tests if the specified OBB is within the frustum.
     *
     * @param bounds The OBB to test.
     * @param t The transformation to apply to the OBB.
     * @return true if the OBB is within the frustum, false otherwise.
     */
    auto test_obb(const math::bbox& bounds, const math::transform& t) const -> bool;

    /**
     * @brief Converts the specified screen position into a ray origin and direction vector.
     *
     * @param point The screen position.
     * @param rayOriginOut The output ray origin.
     * @param rayDirectionOut The output ray direction.
     * @return true if the conversion is successful, false otherwise.
     */
    auto viewport_to_ray(const math::vec2& point, math::vec3& rayOriginOut, math::vec3& rayDirectionOut) const -> bool;

    /**
     * @brief Converts a screen position into a world space position on the specified plane.
     *
     * @param point The screen position.
     * @param plane The plane to intersect.
     * @param positionOut The output world space position.
     * @param clip Whether to clip the result.
     * @return true if the conversion is successful, false otherwise.
     */
    auto viewport_to_world(const math::vec2& point, const math::plane& plane, math::vec3& positionOut, bool clip) const
        -> bool;

    /**
     * @brief Converts a screen position into a world space intersection point on a major axis plane.
     *
     * @param point The screen position.
     * @param axisOrigin The origin of the axis.
     * @param positionOut The output world space position.
     * @param majorAxisOut The output major axis.
     * @return true if the conversion is successful, false otherwise.
     */
    auto viewport_to_major_axis(const math::vec2& point,
                                const math::vec3& axisOrigin,
                                math::vec3& positionOut,
                                math::vec3& majorAxisOut) const -> bool;

    /**
     * @brief Converts a screen position into a world space intersection point on a major axis plane.
     *
     * @param point The screen position.
     * @param axisOrigin The origin of the axis.
     * @param alignNormal The alignment normal.
     * @param positionOut The output world space position.
     * @param majorAxisOut The output major axis.
     * @return true if the conversion is successful, false otherwise.
     */
    auto viewport_to_major_axis(const math::vec2& point,
                                const math::vec3& axisOrigin,
                                const math::vec3& alignNormal,
                                math::vec3& positionOut,
                                math::vec3& majorAxisOut) const -> bool;

    /**
     * @brief Converts a screen position into a camera space position at the near plane.
     *
     * @param point The screen position.
     * @param positionOut The output camera space position.
     * @return true if the conversion is successful, false otherwise.
     */
    auto viewport_to_camera(const math::vec3& point, math::vec3& positionOut) const -> bool;

    /**
     * @brief Transforms a point from world space into screen space.
     *
     * @param pos The world space position.
     * @return The screen space position.
     */
    auto world_to_viewport(const math::vec3& pos) const -> math::vec3;

    /**
     * @brief Estimates the zoom factor based on the specified plane.
     *
     * @param plane The reference plane.
     * @return The estimated zoom factor.
     */
    auto estimate_zoom_factor(const math::plane& plane) const -> float;

    /**
     * @brief Estimates the zoom factor based on the specified position.
     *
     * @param position The reference position.
     * @return The estimated zoom factor.
     */
    auto estimate_zoom_factor(const math::vec3& position) const -> float;

    /**
     * @brief Estimates the zoom factor based on the specified plane, constrained by a maximum value.
     *
     * @param plane The reference plane.
     * @param maximumValue The maximum zoom factor value.
     * @return The estimated zoom factor.
     */
    auto estimate_zoom_factor(const math::plane& plane, float maximumValue) const -> float;

    /**
     * @brief Estimates the zoom factor based on the specified position, constrained by a maximum value.
     *
     * @param position The reference position.
     * @param maximumValue The maximum zoom factor value.
     * @return The estimated zoom factor.
     */
    auto estimate_zoom_factor(const math::vec3& position, float maximumValue) const -> float;

    /**
     * @brief Estimates the pick tolerance based on the pixel tolerance and reference position.
     *
     * @param pixelTolerance The pixel tolerance.
     * @param referencePosition The reference position.
     * @param objectTransform The transformation to apply to the object.
     * @return The estimated pick tolerance.
     */
    auto estimate_pick_tolerance(float pixelTolerance,
                                 const math::vec3& referencePosition,
                                 const math::transform& objectTransform) const -> math::vec3;

    /**
     * @brief Sets the camera to look at a specified target.
     *
     * @param vEye The eye position.
     * @param vAt The target position.
     */
    void look_at(const math::vec3& vEye, const math::vec3& vAt);

    /**
     * @brief Sets the camera to look at a specified target with an up vector.
     *
     * @param vEye The eye position.
     * @param vAt The target position.
     * @param vUp The up vector.
     */
    void look_at(const math::vec3& vEye, const math::vec3& vAt, const math::vec3& vUp);

    /**
     * @brief Retrieves the current position of the camera.
     *
     * @return The current camera position.
     */
    auto get_position() const -> const math::vec3&;

    /**
     * @brief Retrieves the x-axis unit vector of the camera's local coordinate system.
     *
     * @return The x-axis unit vector.
     */
    auto x_unit_axis() const -> math::vec3;

    /**
     * @brief Retrieves the y-axis unit vector of the camera's local coordinate system.
     *
     * @return The y-axis unit vector.
     */
    auto y_unit_axis() const -> math::vec3;

    /**
     * @brief Retrieves the z-axis unit vector of the camera's local coordinate system.
     *
     * @return The z-axis unit vector.
     */
    auto z_unit_axis() const -> math::vec3;

    /**
     * @brief Retrieves the bounding box of this object.
     *
     * @return The local bounding box.
     */
    auto get_local_bounding_box() -> math::bbox;

    /**
     * @brief Marks the camera as modified.
     */
    void touch();

    /**
     * @brief Retrieves a camera for one of six cube faces.
     *
     * @param face The index of the cube face.
     * @param transform The transformation to apply.
     * @return The corresponding face camera.
     */
    static auto get_face_camera(std::uint32_t face, const math::transform& transform) -> camera;

protected:
    /// Anti-aliasing data.
    math::vec4 aa_data_ = {0.0f, 0.0f, 0.0f, 0.0f};
    /// Cached view matrix
    math::transform view_;
    math::transform view_inverse_;
    /// Cached projection matrix.
    mutable math::transform projection_;
    /// Cached "previous" view matrix.
    math::transform last_view_;
    /// Cached "previous" projection matrix.
    math::transform last_projection_;
    /// Details regarding the camera frustum.
    mutable math::frustum frustum_;
    /// The near clipping volume (area of space between the camera position and the near plane).
    mutable math::frustum clipping_volume_;
    /// The type of projection currently selected for this camera.
    projection_mode projection_mode_ = projection_mode::perspective;
    /// Vertical degrees angle (perspective only).
    float fov_ = 60.0f;
    /// Near clip plane Distance
    float near_clip_ = 0.1f;
    /// Far clip plane Distance
    float far_clip_ = 1000.0f;
    /// camera's half-size when in orthographic mode.
    float ortho_size_ = 5;
    /// The aspect ratio used to generate the correct horizontal degrees (perspective only)
    float aspect_ratio_ = 1.0f;
    /// Viewport position
    upoint32_t viewport_pos_ = {0, 0};
    /// Viewport size
    usize32_t viewport_size_ = {0, 0};
    /// View matrix dirty ?
    bool view_dirty_ = true;
    /// Projection matrix dirty ?
    mutable bool projection_dirty_ = true;
    /// Has the aspect ratio changed?
    mutable bool aspect_dirty_ = true;
    /// Are the frustum planes dirty ?
    mutable bool frustum_dirty_ = true;
    /// Should the aspect ratio be automatically updated by the render driver?
    bool aspect_locked_ = false;
    /// Is the frustum locked?
    bool frustum_locked_ = false;
};
} // namespace ace
