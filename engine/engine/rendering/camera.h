#pragma once

#include <base/basetypes.hpp>
#include <context/context.hpp>
#include <math/math.h>
#include <reflection/registration.h>
#include <serialization/serialization.h>

namespace ace
{
enum class projection_mode : std::uint32_t
{
    perspective = 0,
    orthographic = 1
};

struct camera_storage
{
    rtti::context ctx;
};

//-----------------------------------------------------------------------------
// Main Class Declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : camera (Class)
/// <summary>
/// Class representing a camera. Contains functionality for manipulating and
/// updating a camera. It should not be used as a standalone class - see
/// camera_component and the entity system.
/// </summary>
//-----------------------------------------------------------------------------
class camera
{
public:
    REFLECTABLE(camera)
    SERIALIZABLE(camera)

    //-------------------------------------------------------------------------
    // Public Methods
    //-------------------------------------------------------------------------

    //-----------------------------------------------------------------------------
    //  Name : set_projection_mode ()
    /// <summary>
    /// Sets the current projection mode for this camera (i.e. orthographic
    /// or perspective).
    /// </summary>
    //-----------------------------------------------------------------------------
    void set_projection_mode(projection_mode mode);

    //-----------------------------------------------------------------------------
    //  Name : set_fov ()
    /// <summary>
    /// Sets the field of view angle of this camera (perspective only).
    /// </summary>
    //-----------------------------------------------------------------------------
    void set_fov(float degrees);

    //-----------------------------------------------------------------------------
    //  Name : set_near_clip ()
    /// <summary>
    /// Set the near plane distance
    /// </summary>
    //-----------------------------------------------------------------------------
    void set_near_clip(float distance);

    //-----------------------------------------------------------------------------
    //  Name : set_far_clip()
    /// <summary>
    /// Set the far plane distance
    /// </summary>
    //-----------------------------------------------------------------------------
    void set_far_clip(float distance);

    //-----------------------------------------------------------------------------
    // Name : set_orthographic_size( )
    /// <summary>
    /// Sets the half of the vertical size of the viewing volume in world units.
    /// </summary>
    //-----------------------------------------------------------------------------
    void set_orthographic_size(float size);

    //-----------------------------------------------------------------------------
    //  Name : get_projection_mode ()
    /// <summary>
    /// Retrieve the current projection mode for this camera.
    /// </summary>
    //-----------------------------------------------------------------------------
    auto get_projection_mode() const -> projection_mode;

    //-----------------------------------------------------------------------------
    //  Name : get_fov()
    /// <summary>
    /// Retrieve the current field of view angle in degrees.
    /// </summary>
    //-----------------------------------------------------------------------------
    auto get_fov() const -> float;

    //-----------------------------------------------------------------------------
    //  Name : get_near_clip()
    /// <summary>
    /// Retrieve the distance from the camera to the near clip plane.
    /// </summary>
    //-----------------------------------------------------------------------------
    auto get_near_clip() const -> float;

    //-----------------------------------------------------------------------------
    //  Name : get_far_clip()
    /// <summary>
    /// Retrieve the distance from the camera to the far clip plane.
    /// </summary>
    //-----------------------------------------------------------------------------
    auto get_far_clip() const -> float;

    //-----------------------------------------------------------------------------
    // Name : get_ortho_size( )
    /// <summary>
    /// Get the orthographic size.
    /// </summary>
    //-----------------------------------------------------------------------------
    auto get_ortho_size() const -> float;

    //-----------------------------------------------------------------------------
    //  Name : get_zoom_factor ()
    /// <summary>
    ///
    ///
    ///
    /// </summary>
    //-----------------------------------------------------------------------------
    float get_zoom_factor() const;

    //-----------------------------------------------------------------------------
    //  Name : get_ppu ()
    /// <summary>
    ///
    ///
    ///
    /// </summary>
    //-----------------------------------------------------------------------------
    float get_ppu() const;

    //-----------------------------------------------------------------------------
    //  Name : set_viewport_size ()
    /// <summary>
    ///
    ///
    ///
    /// </summary>
    //-----------------------------------------------------------------------------
    void set_viewport_size(const usize32_t& viewportSize);

    //-----------------------------------------------------------------------------
    //  Name : set_viewport_pos ()
    /// <summary>
    ///
    ///
    ///
    /// </summary>
    //-----------------------------------------------------------------------------
    void set_viewport_pos(const upoint32_t& viewportPos);

    //-----------------------------------------------------------------------------
    //  Name : get_viewport_size ()
    /// <summary>
    ///
    ///
    ///
    /// </summary>
    //-----------------------------------------------------------------------------
    auto get_viewport_size() const -> const usize32_t&;

    //-----------------------------------------------------------------------------
    //  Name : get_viewport_pos ()
    /// <summary>
    ///
    ///
    ///
    /// </summary>
    //-----------------------------------------------------------------------------
    auto get_viewport_pos() const -> const upoint32_t&;

    //-----------------------------------------------------------------------------
    //  Name : set_aspect_ratio ()
    /// <summary>
    /// Set the aspect ratio that should be used to generate the horizontal
    /// FOV angle (perspective only).
    /// </summary>
    //-----------------------------------------------------------------------------
    void set_aspect_ratio(float aspect, bool locked = false);

    //-----------------------------------------------------------------------------
    //  Name : get_aspect_ratio()
    /// <summary>
    /// Retrieve the aspect ratio used to generate the horizontal FOV angle.
    /// </summary>
    //-----------------------------------------------------------------------------
    auto get_aspect_ratio() const -> float;

    //-----------------------------------------------------------------------------
    //  Name : is_aspect_locked()
    /// <summary>
    /// Determine if the aspect ratio is currently being updated by the
    /// render driver.
    /// </summary>
    //-----------------------------------------------------------------------------
    bool is_aspect_locked() const;

    //-----------------------------------------------------------------------------
    //  Name : is_frustum_locked ()
    /// <summary>
    /// Inform the caller whether or not the frustum is currently locked
    /// This is useful as a debugging tool.
    /// </summary>
    //-----------------------------------------------------------------------------
    auto is_frustum_locked() const -> bool;

    //-----------------------------------------------------------------------------
    //  Name : lock_frustum ()
    /// <summary>
    /// Prevent the frustum from updating.
    /// </summary>
    //-----------------------------------------------------------------------------
    void lock_frustum(bool locked);

    //-----------------------------------------------------------------------------
    //  Name : get_frustum()
    /// <summary>
    /// Retrieve the current camera object frustum.
    /// </summary>
    //-----------------------------------------------------------------------------
    const math::frustum& get_frustum() const;

    //-----------------------------------------------------------------------------
    //  Name : get_clipping_volume()
    /// <summary>
    /// Retrieve the frustum / volume that represents the space between the camera
    /// position and its near plane. This frustum represents the 'volume' that can
    /// end up clipping geometry.
    /// </summary>
    //-----------------------------------------------------------------------------
    const math::frustum& get_clipping_volume() const;

    //-----------------------------------------------------------------------------
    //  Name : get_projection ()
    /// <summary>
    /// Return the current projection matrix.
    /// </summary>
    //-----------------------------------------------------------------------------
    const math::transform& get_projection() const;

    //-----------------------------------------------------------------------------
    //  Name : get_view ()
    /// <summary>
    /// Return the current view matrix.
    /// </summary>
    //-----------------------------------------------------------------------------
    auto get_view() const -> const math::transform&;

    //-----------------------------------------------------------------------------
    //  Name : get_view_projection ()
    /// <summary>
    /// Return the current view-projection matrix.
    /// </summary>
    //-----------------------------------------------------------------------------
    auto get_view_projection() const -> math::transform;

    //-----------------------------------------------------------------------------
    //  Name : record_current_matrices ()
    /// <summary>
    /// Make a copy of the current view / projection matrices before they
    /// are changed. Useful for performing effects such as motion blur.
    /// </summary>
    //-----------------------------------------------------------------------------
    void record_current_matrices();

    //-----------------------------------------------------------------------------
    //  Name : set_aa_data ()
    /// <summary>
    /// Sets the current jitter value for temporal anti-aliasing
    /// </summary>
    //-----------------------------------------------------------------------------
    void set_aa_data(const usize32_t& viewportSize,
                     std::uint32_t currentSubpixelIndex,
                     std::uint32_t temporalAASamples);

    //-----------------------------------------------------------------------------
    //  Name : get_aa_data ()
    /// <summary>
    ///
    ///
    ///
    /// </summary>
    //-----------------------------------------------------------------------------
    auto get_aa_data() const -> const math::vec4&;

    //-----------------------------------------------------------------------------
    //  Name : bounds_in_frustum ()
    /// <summary>
    /// Determine whether or not the AABB specified falls within the frustum.
    /// </summary>
    //-----------------------------------------------------------------------------
    auto classify_aabb(const math::bbox& bounds) const -> math::volume_query;
    auto test_aabb(const math::bbox& bounds) const -> bool;

    //-----------------------------------------------------------------------------
    //  Name : bounds_in_frustum ()
    /// <summary>
    /// Determine whether or not the OOBB specified is within the frustum.
    /// </summary>
    //-----------------------------------------------------------------------------
    auto classify_obb(const math::bbox& bounds, const math::transform& t) const -> math::volume_query;
    auto test_obb(const math::bbox& bounds, const math::transform& t) const -> bool;

    //-----------------------------------------------------------------------------
    //  Name : viewport_to_ray()
    /// <summary>
    /// Convert the specified screen position into a ray origin and direction
    /// vector, suitable for use during picking.
    /// </summary>
    //-----------------------------------------------------------------------------
    auto viewport_to_ray(const math::vec2& point, math::vec3& rayOriginOut, math::vec3& rayDirectionOut) const -> bool;

    //-----------------------------------------------------------------------------
    //  Name : viewport_to_world ()
    /// <summary>
    /// Given a view screen position (in screen space) this function will cast
    /// that
    /// ray and return the world space position on the specified plane. The value
    /// is returned via the world parameter passed.
    /// </summary>
    //-----------------------------------------------------------------------------
    auto viewport_to_world(const math::vec2& point, const math::plane& plane, math::vec3& positionOut, bool clip) const
        -> bool;

    //-----------------------------------------------------------------------------
    //  Name : viewport_to_major_axis ()
    /// <summary>
    /// Given a view screen position (in screen space) this function will cast
    /// that
    /// ray and return the world space intersection point on one of the major axis
    /// planes selected based on the camera look vector.
    /// </summary>
    //-----------------------------------------------------------------------------
    auto viewport_to_major_axis(const math::vec2& point,
                                const math::vec3& axisOrigin,
                                math::vec3& positionOut,
                                math::vec3& majorAxisOut) const -> bool;

    //-----------------------------------------------------------------------------
    //  Name : viewportToMajorAxis ()
    /// <summary>
    /// Given a view screen position (in screen space) this function will cast
    /// that
    /// ray and return the world space intersection point on one of the major axis
    /// planes selected based on the specified normal.
    /// </summary>
    //-----------------------------------------------------------------------------
    auto viewport_to_major_axis(const math::vec2& point,
                                const math::vec3& axisOrigin,
                                const math::vec3& alignNormal,
                                math::vec3& positionOut,
                                math::vec3& majorAxisOut) const -> bool;

    //-----------------------------------------------------------------------------
    //  Name : viewport_to_camera ()
    /// <summary>
    /// Given a view screen position (in screen space) this function will convert
    /// the point into a camera space position at the near plane.
    /// </summary>
    //-----------------------------------------------------------------------------
    auto viewport_to_camera(const math::vec3& point, math::vec3& positionOut) const -> bool;

    //-----------------------------------------------------------------------------
    //  Name : world_to_viewport()
    /// <summary>
    /// Transform a point from world space, into screen space. Returns false
    /// if the point was clipped off the screen.
    /// </summary>
    //-----------------------------------------------------------------------------
    auto world_to_viewport(const math::vec3& pos) const -> math::vec3;

    //-----------------------------------------------------------------------------
    //  Name : estimate_zoom_factor ()
    /// <summary>
    ///
    ///
    ///
    /// </summary>
    //-----------------------------------------------------------------------------
    auto estimate_zoom_factor(const math::plane& plane) const -> float;

    //-----------------------------------------------------------------------------
    //  Name : estimate_zoom_factor ()
    /// <summary>
    /// Given the current viewport type and projection mode, estimate the "zoom"
    /// factor that can be used for scaling various operations relative to their
    /// "scale" as it appears in the viewport.
    /// </summary>
    //-----------------------------------------------------------------------------
    auto estimate_zoom_factor(const math::vec3& position) const -> float;

    //-----------------------------------------------------------------------------
    //  Name : estimate_zoom_factor ()
    /// <summary>
    /// Given the current viewport type and projection mode, estimate the "zoom"
    /// factor that can be used for scaling various operations relative to their
    /// "scale" as it appears in the viewport.
    /// </summary>
    //-----------------------------------------------------------------------------
    auto estimate_zoom_factor(const math::plane& plane, float maximumValue) const -> float;

    //-----------------------------------------------------------------------------
    // Name : estimate_zoom_factor ()
    /// <summary>
    /// Given the current viewport type and projection mode, estimate the "zoom"
    /// factor that can be used for scaling various operations relative to the
    /// "scale" of an object as it appears in the viewport at the specified
    /// position.
    /// </summary>
    //-----------------------------------------------------------------------------
    auto estimate_zoom_factor(const math::vec3& position, float maximumValue) const -> float;

    //-----------------------------------------------------------------------------
    // Name : estimate_pick_tolerance ()
    /// <summary>
    /// Estimate the distance (along each axis) from the specified object space
    /// point to use as a tolerance for picking.
    /// </summary>
    //-----------------------------------------------------------------------------
    auto estimate_pick_tolerance(float pixelTolerance,
                                 const math::vec3& referencePosition,
                                 const math::transform& objectTransform) const -> math::vec3;

    //-----------------------------------------------------------------------------
    //  Name : look_at ()
    /// <summary>
    ///
    ///
    ///
    /// </summary>
    //-----------------------------------------------------------------------------
    void look_at(const math::vec3& vEye, const math::vec3& vAt);

    //-----------------------------------------------------------------------------
    //  Name : look_at ()
    /// <summary>
    ///
    ///
    ///
    /// </summary>
    //-----------------------------------------------------------------------------
    void look_at(const math::vec3& vEye, const math::vec3& vAt, const math::vec3& vUp);

    //-----------------------------------------------------------------------------
    //  Name : get_position ()
    /// <summary>
    ///
    ///
    ///
    /// </summary>
    //-----------------------------------------------------------------------------
    auto get_position() const -> const math::vec3&;

    //-----------------------------------------------------------------------------
    //  Name : x_unit_axis ()
    /// <summary>
    ///
    ///
    ///
    /// </summary>
    //-----------------------------------------------------------------------------
    auto x_unit_axis() const -> math::vec3;

    //-----------------------------------------------------------------------------
    //  Name : y_unit_axis ()
    /// <summary>
    ///
    ///
    ///
    /// </summary>
    //-----------------------------------------------------------------------------
    auto y_unit_axis() const -> math::vec3;

    //-----------------------------------------------------------------------------
    //  Name : z_unit_axis ()
    /// <summary>
    ///
    ///
    ///
    /// </summary>
    //-----------------------------------------------------------------------------
    auto z_unit_axis() const -> math::vec3;

    //-----------------------------------------------------------------------------
    //  Name : get_local_bounding_box ()
    /// <summary>
    /// Retrieve the bounding box of this object.
    /// </summary>
    //-----------------------------------------------------------------------------
    auto get_local_bounding_box() -> math::bbox;

    //-----------------------------------------------------------------------------
    //  Name : () touch
    /// <summary>
    /// When the camera is modified.
    /// </summary>
    //-----------------------------------------------------------------------------
    void touch();

    //-----------------------------------------------------------------------------
    //  Name : () touch
    /// <summary>
    /// Get camera for one of six cube faces
    /// </summary>
    //-----------------------------------------------------------------------------
    static auto get_face_camera(std::uint32_t face, const math::transform& transform) -> camera;

protected:
    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
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
    /// The near clipping volume (area of space between the camera position and
    /// the near plane).
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
    /// The aspect ratio used to generate the correct horizontal degrees
    /// (perspective only)
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
