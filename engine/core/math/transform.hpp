#pragma once
//-----------------------------------------------------------------------------
// transform Header Includes
//-----------------------------------------------------------------------------
#include "matrix_recompose.hpp"

namespace math
{
using namespace glm;

//-----------------------------------------------------------------------------
// Main class declarations
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//  Name : transform (Class)
/// <summary>
/// General purpose transformation class designed to maintain each component of
/// the transformation separate (translation, rotation, scale and shear) whilst
/// providing much of the same functionality provided by standard matrices.
/// </summary>
//-----------------------------------------------------------------------------
template<typename T, precision Q = defaultp>
class transform_t
{
public:
    using mat3_t = mat<3, 3, T, Q>;
    using mat4_t = mat<4, 4, T, Q>;
    using vec2_t = vec<2, T, Q>;
    using vec3_t = vec<3, T, Q>;
    using vec4_t = vec<4, T, Q>;
    using quat_t = qua<T, Q>;

    //-------------------------------------------------------------------------
    // Constructors & Destructors
    //-------------------------------------------------------------------------

    /**
     * @brief Default constructor.
     */
    transform_t() = default;

    /**
     * @brief Copy constructor.
     */
    transform_t(const transform_t& t) = default;

    /**
     * @brief Move constructor.
     */
    transform_t(transform_t&& t) noexcept = default;

    /**
     * @brief Copy assignment operator.
     */
    auto operator=(const transform_t& m) -> transform_t& = default;

    /**
     * @brief Move assignment operator.
     */
    auto operator=(transform_t&& m) noexcept -> transform_t& = default;

    /**
     * @brief Constructor from a 4x4 matrix.
     */
    transform_t(const mat4_t& m) noexcept;

    /**
     * @brief Get the position component.
     * @return A constant reference to the position vector.
     */
    auto get_position() const noexcept -> const vec3_t&;

    /**
     * @brief Get the translation component.
     * @return A constant reference to the translation vector.
     */
    auto get_translation() const noexcept -> const vec3_t&;

    /**
     * @brief Set the translation component.
     */
    void set_translation(const vec3_t& position) noexcept;

    /**
     * @brief Set the translation component.
     */
    void set_translation(T x, T y, T z = T(0)) noexcept;

    /**
     * @brief Reset the translation component to zero.
     */
    void reset_translation() noexcept;

    /**
     * @brief Set the position component.
     */
    void set_position(const vec3_t& position) noexcept;

    /**
     * @brief Set the position component.
     */
    void set_position(T x, T y, T z = T(0)) noexcept;

    /**
     * @brief Reset the position component to zero.
     */
    void reset_position() noexcept;

    /**
     * @brief Get the rotation component as Euler angles.
     * @return The rotation component as Euler angles.
     */
    auto get_rotation_euler() const noexcept -> vec3_t;

    /**
     * @brief Get the rotation component as Euler angles in degrees.
     * @return The rotation component as Euler angles in degrees.
     */
    auto get_rotation_euler_degrees() const noexcept -> vec3_t;

    /**
     * @brief Get the rotation component as Euler angles in degrees with a hint.
     * @return The rotation component as Euler angles in degrees.
     */
    auto get_rotation_euler_degrees(vec3_t hint) const noexcept -> vec3_t;

    /**
     * @brief Get the rotation component as Euler angles in degrees with a hint (BL method).
     * @return The rotation component as Euler angles in degrees.
     */
    auto get_rotation_euler_degrees_bl(vec3_t hint) const noexcept -> vec3_t;

    /**
     * @brief Set the rotation component using Euler angles.
     */
    void set_rotation_euler(const vec3_t& euler_angles) noexcept;

    /**
     * @brief Set the rotation component using Euler angles.
     */
    void set_rotation_euler(T x, T y, T z) noexcept;

    /**
     * @brief Set the rotation component using Euler angles in degrees.
     */
    void set_rotation_euler_degrees(const vec3_t& euler_angles) noexcept;

    /**
     * @brief Set the rotation component using Euler angles in degrees.
     */
    void set_rotation_euler_degrees(T x, T y, T z) noexcept;

    /**
     * @brief Get the scale component.
     * @return A constant reference to the scale vector.
     */
    auto get_scale() const noexcept -> const vec3_t&;

    /**
     * @brief Set the scale component.
     */
    void set_scale(const vec3_t& scale) noexcept;

    /**
     * @brief Set the scale component.
     */
    void set_scale(T x, T y, T z = T(1)) noexcept;

    /**
     * @brief Reset the scale component to one.
     */
    void reset_scale() noexcept;

    /**
     * @brief Get the rotation component.
     * @return A constant reference to the quaternion rotation.
     */
    auto get_rotation() const noexcept -> const quat_t&;

    /**
     * @brief Set the rotation component.
     */
    void set_rotation(const quat_t& rotation) noexcept;

    /**
     * @brief Set the rotation component using basis vectors.
     */
    void set_rotation(const vec3_t& x, const vec3_t& y, const vec3_t& z) noexcept;

    /**
     * @brief Reset the rotation component to identity.
     */
    void reset_rotation() noexcept;

    /**
     * @brief Get the skew component.
     * @return A constant reference to the skew vector.
     */
    auto get_skew() const noexcept -> const vec3_t&;

    /**
     * @brief Set the skew component.
     */
    void set_skew(const vec3_t& skew) noexcept;

    /**
     * @brief Set the skew component.
     */
    void set_skew(T x, T y, T z) noexcept;

    /**
     * @brief Reset the skew component to zero.
     */
    void reset_skew() noexcept;

    /**
     * @brief Get the perspective component.
     * @return A constant reference to the perspective vector.
     */
    auto get_perspective() const noexcept -> const vec4_t&;

    /**
     * @brief Set the perspective component.
     */
    void set_perspective(const vec4_t& perspective) noexcept;

    /**
     * @brief Set the perspective component.
     */
    void set_perspective(T x, T y, T z, T w) noexcept;

    /**
     * @brief Reset the perspective component to default.
     */
    void reset_perspective() noexcept;

    /**
     * @brief Get the X axis of the transform.
     * @return The X axis vector.
     */
    auto x_axis() const noexcept -> vec3_t;

    /**
     * @brief Get the Y axis of the transform.
     * @return The Y axis vector.
     */
    auto y_axis() const noexcept -> vec3_t;

    /**
     * @brief Get the Z axis of the transform.
     * @return The Z axis vector.
     */
    auto z_axis() const noexcept -> vec3_t;

    /**
     * @brief Get the unit X axis of the transform.
     * @return The unit X axis vector.
     */
    auto x_unit_axis() const noexcept -> vec3_t;

    /**
     * @brief Get the unit Y axis of the transform.
     * @return The unit Y axis vector.
     */
    auto y_unit_axis() const noexcept -> vec3_t;

    /**
     * @brief Get the unit Z axis of the transform.
     * @return The unit Z axis vector.
     */
    auto z_unit_axis() const noexcept -> vec3_t;

    // these transform from the current state
    /**
     * @brief Rotate the transform around an axis.
     */
    void rotate_axis(T a, const vec3_t& v) noexcept;

    /**
     * @brief Rotate the transform using a quaternion.
     */
    void rotate(const quat_t& q) noexcept;

    /**
     * @brief Rotate the transform using Euler angles.
     */
    void rotate(T x, T y, T z) noexcept;

    /**
     * @brief Rotate the transform using Euler angles.
     */
    void rotate(const vec3_t& v) noexcept;

    /**
     * @brief Rotate the transform locally using Euler angles.
     */
    void rotate_local(T x, T y, T z) noexcept;

    /**
     * @brief Rotate the transform locally using Euler angles.
     */
    void rotate_local(const vec3_t& v) noexcept;

    /**
     * @brief Scale the transform.
     */
    void scale(T x, T y, T z = T(1)) noexcept;

    /**
     * @brief Scale the transform.
     */
    void scale(const vec3_t& v) noexcept;

    /**
     * @brief Translate the transform.
     */
    void translate(T x, T y, T z = T(0)) noexcept;

    /**
     * @brief Translate the transform.
     */
    void translate(const vec3_t& v) noexcept;

    /**
     * @brief Translate the transform locally.
     */
    void translate_local(T x, T y, T z = T(0)) noexcept;

    /**
     * @brief Translate the transform locally.
     */
    void translate_local(const vec3_t& v) noexcept;

    /**
     * @brief Compare this transform with another.
     * @return -1, 0, or 1 based on comparison.
     */
    auto compare(const transform_t& t) const noexcept -> int;

    /**
     * @brief Compare this transform with another within a tolerance.
     * @return -1, 0, or 1 based on comparison.
     */
    auto compare(const transform_t& t, T tolerance) const noexcept -> int;

    /**
     * @brief Transform a 2D coordinate.
     * @return The transformed 2D coordinate.
     */
    auto transform_coord(const vec2_t& v) const noexcept -> vec2_t;

    /**
     * @brief Inverse transform a 2D coordinate.
     * @return The inverse transformed 2D coordinate.
     */
    auto inverse_transform_coord(const vec2_t& v) const noexcept -> vec2_t;

    /**
     * @brief Transform a 2D normal.
     * @return The transformed 2D normal.
     */
    auto transform_normal(const vec2_t& v) const noexcept -> vec2_t;

    /**
     * @brief Inverse transform a 2D normal.
     * @return The inverse transformed 2D normal.
     */
    auto inverse_transform_normal(const vec2_t& v) const noexcept -> vec2_t;

    /**
     * @brief Transform a 3D coordinate.
     * @return The transformed 3D coordinate.
     */
    auto transform_coord(const vec3_t& v) const noexcept -> vec3_t;

    /**
     * @brief Inverse transform a 3D coordinate.
     * @return The inverse transformed 3D coordinate.
     */
    auto inverse_transform_coord(const vec3_t& v) const noexcept -> vec3_t;

    /**
     * @brief Transform a 3D normal.
     * @return The transformed 3D normal.
     */
    auto transform_normal(const vec3_t& v) const noexcept -> vec3_t;

    /**
     * @brief Inverse transform a 3D normal.
     * @return The inverse transformed 3D normal.
     */
    auto inverse_transform_normal(const vec3_t& v) const noexcept -> vec3_t;

    /**
     * @brief Get the identity transform.
     * @return A constant reference to the identity transform.
     */
    static auto identity() noexcept -> const transform_t&;

    /**
     * @brief Create a scaling transform.
     * @return The scaling transform.
     */
    static auto scaling(const vec2_t& scale) -> transform_t;

    /**
     * @brief Create a scaling transform.
     * @return The scaling transform.
     */
    static auto scaling(const vec3_t& scale) -> transform_t;

    /**
     * @brief Create a rotation transform.
     * @return The rotation transform.
     */
    static auto rotation(const quat_t& rotation) -> transform_t;

    /**
     * @brief Create a rotation transform from Euler angles.
     * @return The rotation transform.
     */
    static auto rotation_euler(const vec3_t& euler_angles) -> transform_t;

    /**
     * @brief Create a translation transform.
     * @return The translation transform.
     */
    static auto translation(const vec2_t& trans) -> transform_t;

    /**
     * @brief Create a translation transform.
     * @return The translation transform.
     */
    static auto translation(const vec3_t& trans) -> transform_t;

    //-------------------------------------------------------------------------
    // Public Operator Overloads
    //-------------------------------------------------------------------------
    /**
     * @brief Implicit conversion to a constant reference to the transformation matrix.
     *
     * Allows the transform to be used wherever a constant reference to a mat4_t is expected.
     *
     * @return A constant reference to the transformation matrix.
     */
    operator const mat4_t&() const noexcept;

    /**
     * @brief Implicit conversion to a constant pointer to the transformation matrix.
     *
     * Allows the transform to be used wherever a constant pointer to a mat4_t is expected.
     *
     * @return A constant pointer to the transformation matrix.
     */
    operator const mat4_t*() const noexcept;

    /**
     * @brief Implicit conversion to a constant pointer to the transformation matrix data.
     *
     * Allows the transform to be used wherever a constant pointer to the underlying data is expected,
     * facilitating interfacing with APIs that require raw matrix data.
     *
     * @return A constant pointer to the transformation matrix data.
     */
    operator const T*() const noexcept;

    /**
     * @brief Multiply two transforms.
     * @return The result of the multiplication.
     */
    auto operator*(const transform_t& t) const noexcept -> transform_t;

    /**
     * @brief Compare two transforms for equality.
     * @return True if equal, false otherwise.
     */
    auto operator==(const transform_t& t) const noexcept -> bool;

    /**
     * @brief Compare two transforms for inequality.
     * @return True if not equal, false otherwise.
     */
    auto operator!=(const transform_t& t) const noexcept -> bool;

    /**
     * @brief Get a column of the transform matrix.
     * @return The column of the matrix.
     */
    auto operator[](typename mat4_t::length_type i) const noexcept -> const typename mat4_t::col_type&;

    /**
     * @brief Transform a 4D vector.
     * @return The transformed 4D vector.
     */
    auto operator*(const vec4_t& v) const noexcept -> vec4_t;

    /**
     * @brief Get the transform matrix.
     * @return A constant reference to the transform matrix.
     */
    auto get_matrix() const noexcept -> const mat4_t&;

private:
    void update_components() noexcept;
    void update_matrix() const noexcept;
    void make_dirty() noexcept;

    //-------------------------------------------------------------------------
    // Protected Variables
    //-------------------------------------------------------------------------
    // this should be always first.
    mutable mat4_t matrix_ = glm::identity<mat4_t>();

    vec3_t position_ = glm::zero<vec3_t>();
    quat_t rotation_ = glm::identity<quat_t>();
    vec3_t scale_ = glm::one<vec3_t>();
    vec3_t skew_ = glm::zero<vec3_t>();
    vec4_t perspective_ = vec4_t(0, 0, 0, 1);
    mutable bool dirty_ = false;
};

template<typename T, precision Q>
auto inverse(transform_t<T, Q> const& t) noexcept -> transform_t<T, Q>
{
    const auto& m = t.get_matrix();
    return glm::inverse(m);
}

template<typename T, precision Q>
auto transpose(transform_t<T, Q> const& t) noexcept -> transform_t<T, Q>
{
    const auto& m = t.get_matrix();
    return glm::transpose(m);
}

template<typename T, precision Q>
inline transform_t<T, Q>::transform_t(const mat4_t& m) noexcept : matrix_(m)
{
    update_components();
}

template<typename T, precision Q>
inline auto transform_t<T, Q>::get_position() const noexcept -> const typename transform_t<T, Q>::vec3_t&
{
    return position_;
}

template<typename T, precision Q>
inline auto transform_t<T, Q>::get_translation() const noexcept -> const typename transform_t<T, Q>::vec3_t&
{
    return get_position();
}

template<typename T, precision Q>
inline void transform_t<T, Q>::set_translation(const vec3_t& position) noexcept
{
    set_position(position);
}

template<typename T, precision Q>
inline void transform_t<T, Q>::set_translation(T x, T y, T z) noexcept
{
    set_position(x, y, z);
}

template<typename T, precision Q>
inline void transform_t<T, Q>::reset_translation() noexcept
{
    set_translation(0.0f, 0.0f, 0.0f);
}

template<typename T, precision Q>
inline void transform_t<T, Q>::set_position(const vec3_t& position) noexcept
{
    position_ = position;
    make_dirty();
}

template<typename T, precision Q>
inline void transform_t<T, Q>::set_position(T x, T y, T z) noexcept
{
    set_position({x, y, z});
}

template<typename T, precision Q>
inline void transform_t<T, Q>::reset_position() noexcept
{
    set_position(0.0f, 0.0f, 0.0f);
}

template<typename T, precision Q>
inline auto transform_t<T, Q>::get_rotation_euler() const noexcept -> typename transform_t<T, Q>::vec3_t
{
    return eulerAngles(rotation_);
}

template<typename T, precision Q>
inline auto transform_t<T, Q>::get_rotation_euler_degrees() const noexcept -> typename transform_t<T, Q>::vec3_t
{
    auto angles = degrees(get_rotation_euler());
    return angles;
}

template<typename T, precision Q>
inline auto transform_t<T, Q>::get_rotation_euler_degrees(vec3_t hint) const noexcept ->
    typename transform_t<T, Q>::vec3_t
{
    auto angles = get_rotation_euler_degrees();

    static auto repeat_working = [](float t, float length)
    {
        return (t - (floor(t / length) * length));
    };

    angles.x = repeat_working(angles.x - hint.x + 180.0f, 360.0f) + hint.x - 180.0f;
    angles.y = repeat_working(angles.y - hint.y + 180.0f, 360.0f) + hint.y - 180.0f;
    angles.z = repeat_working(angles.z - hint.z + 180.0f, 360.0f) + hint.z - 180.0f;

    return angles;
}

template<typename T, precision Q>
inline auto transform_t<T, Q>::get_rotation_euler_degrees_bl(vec3_t hint) const noexcept ->
    typename transform_t<T, Q>::vec3_t
{
    auto eul = get_rotation_euler_degrees();

    /* we could use M_PI as pi_thresh: which is correct but 5.1 gives better results.
     * Checked with baking actions to fcurves - campbell */
    eul = radians(eul);
    hint = radians(hint);

    const float pi_thresh = (5.1f);
    const float pi_x2 = (2.0f * (float)math::pi<float>());

    auto dif = vec3_t(0, 0, 0);
    /* correct differences of about 360 degrees first */
    for(int i = 0; i < 3; i++)
    {
        dif[i] = eul[i] - hint[i];
        if(dif[i] > pi_thresh)
        {
            eul[i] -= floor((dif[i] / pi_x2) + 0.5f) * pi_x2;
            dif[i] = eul[i] - hint[i];
        }
        else if(dif[i] < -pi_thresh)
        {
            eul[i] += floor((-dif[i] / pi_x2) + 0.5f) * pi_x2;
            dif[i] = eul[i] - hint[i];
        }
    }

    /* is 1 of the axis rotations larger than 180 degrees and the other small? NO ELSE IF!! */
    if(abs(dif[0]) > 3.2f && abs(dif[1]) < 1.6f && abs(dif[2]) < 1.6f)
    {
        if(dif[0] > 0.0f)
        {
            eul[0] -= pi_x2;
        }
        else
        {
            eul[0] += pi_x2;
        }
    }
    if(abs(dif[1]) > 3.2f && abs(dif[2]) < 1.6f && abs(dif[0]) < 1.6f)
    {
        if(dif[1] > 0.0f)
        {
            eul[1] -= pi_x2;
        }
        else
        {
            eul[1] += pi_x2;
        }
    }
    if(abs(dif[2]) > 3.2f && abs(dif[0]) < 1.6f && abs(dif[1]) < 1.6f)
    {
        if(dif[2] > 0.0f)
        {
            eul[2] -= pi_x2;
        }
        else
        {
            eul[2] += pi_x2;
        }
    }
    return degrees(eul);
}

template<typename T, precision Q>
inline void transform_t<T, Q>::set_rotation_euler(const vec3_t& euler_angles) noexcept
{
    set_rotation(quat_t(euler_angles));
}

template<typename T, precision Q>
inline void transform_t<T, Q>::set_rotation_euler(T x, T y, T z) noexcept
{
    set_rotation_euler({x, y, z});
}

template<typename T, precision Q>
inline void transform_t<T, Q>::set_rotation_euler_degrees(const vec3_t& euler_angles) noexcept
{
    set_rotation_euler(radians(euler_angles));
}

template<typename T, precision Q>
inline void transform_t<T, Q>::set_rotation_euler_degrees(T x, T y, T z) noexcept
{
    set_rotation_euler_degrees({x, y, z});
}

template<typename T, precision Q>
inline auto transform_t<T, Q>::get_scale() const noexcept -> const typename transform_t<T, Q>::vec3_t&
{
    return scale_;
}

template<typename T, precision Q>
inline void transform_t<T, Q>::set_scale(const vec3_t& scale) noexcept
{
    scale_ = scale;
    make_dirty();
}

template<typename T, precision Q>
inline void transform_t<T, Q>::set_scale(T x, T y, T z) noexcept
{
    set_scale({x, y, z});
}

template<typename T, precision Q>
inline void transform_t<T, Q>::reset_scale() noexcept
{
    set_scale(1.0f, 1.0f, 1.0f);
}

template<typename T, precision Q>
inline void transform_t<T, Q>::set_skew(const vec3_t& skew) noexcept
{
    skew_ = skew;
    make_dirty();
}

template<typename T, precision Q>
inline void transform_t<T, Q>::set_skew(T x, T y, T z) noexcept
{
    set_skew({x, y, z});
}

template<typename T, precision Q>
inline void transform_t<T, Q>::reset_skew() noexcept
{
    set_skew(0, 0, 0);
}

template<typename T, precision Q>
inline void transform_t<T, Q>::set_perspective(const vec4_t& perspective) noexcept
{
    perspective_ = perspective;
    make_dirty();
}

template<typename T, precision Q>
inline void transform_t<T, Q>::set_perspective(T x, T y, T z, T w) noexcept
{
    set_perspective({x, y, z, w});
}

template<typename T, precision Q>
inline void transform_t<T, Q>::reset_perspective() noexcept
{
    set_perspective(0, 0, 0, 1);
}

template<typename T, precision Q>
inline auto transform_t<T, Q>::get_rotation() const noexcept -> const typename transform_t<T, Q>::quat_t&
{
    return rotation_;
}

template<typename T, precision Q>
inline auto transform_t<T, Q>::get_skew() const noexcept -> const typename transform_t<T, Q>::vec3_t&
{
    return skew_;
}

template<typename T, precision Q>
inline auto transform_t<T, Q>::get_perspective() const noexcept -> const typename transform_t<T, Q>::vec4_t&
{
    return perspective_;
}

template<typename T, precision Q>
inline void transform_t<T, Q>::set_rotation(const quat_t& rotation) noexcept
{
    rotation_ = glm::normalize(rotation);
    make_dirty();
}

template<typename T, precision Q>
inline void transform_t<T, Q>::set_rotation(const vec3_t& x, const vec3_t& y, const vec3_t& z) noexcept
{
    quat_t quat = quat_cast(mat3_t(x, y, z));
    set_rotation(quat);
}

template<typename T, precision Q>
inline void transform_t<T, Q>::reset_rotation() noexcept
{
    set_rotation(quat_t(1, 0, 0, 0));
}

template<typename T, precision Q>
inline auto transform_t<T, Q>::x_axis() const noexcept -> typename transform_t<T, Q>::vec3_t
{
    return get_matrix()[0];
}

template<typename T, precision Q>
inline auto transform_t<T, Q>::y_axis() const noexcept -> typename transform_t<T, Q>::vec3_t
{
    return get_matrix()[1];
}

template<typename T, precision Q>
inline auto transform_t<T, Q>::z_axis() const noexcept -> typename transform_t<T, Q>::vec3_t
{
    return get_matrix()[2];
}

template<typename T, precision Q>
inline auto transform_t<T, Q>::x_unit_axis() const noexcept -> typename transform_t<T, Q>::vec3_t
{
    return normalize(x_axis());
}

template<typename T, precision Q>
inline auto transform_t<T, Q>::y_unit_axis() const noexcept -> typename transform_t<T, Q>::vec3_t
{
    return normalize(y_axis());
}

template<typename T, precision Q>
inline auto transform_t<T, Q>::z_unit_axis() const noexcept -> typename transform_t<T, Q>::vec3_t
{
    return normalize(z_axis());
}

template<typename T, precision Q>
inline void transform_t<T, Q>::rotate(const quat_t& q) noexcept
{
    quat_t result = q * get_rotation();
    set_rotation(result);
}

template<typename T, precision Q>
inline void transform_t<T, Q>::rotate_axis(T a, const vec3_t& v) noexcept
{
    quat_t q = glm::angleAxis(a, v) * get_rotation();
    set_rotation(q);
}

template<typename T, precision Q>
inline void transform_t<T, Q>::rotate(T x, T y, T z) noexcept
{
    rotate({x, y, z});
}

template<typename T, precision Q>
inline void transform_t<T, Q>::rotate(const vec3_t& v) noexcept
{
    quat_t qx = glm::angleAxis(v.x, vec3_t{1, 0, 0});
    quat_t qy = glm::angleAxis(v.y, vec3_t{0, 1, 0});
    quat_t qz = glm::angleAxis(v.z, vec3_t{0, 0, 1});
    quat_t q = qz * qy * qx * get_rotation();
    set_rotation(q);
}

template<typename T, precision Q>
inline void transform_t<T, Q>::rotate_local(T x, T y, T z) noexcept
{
    rotate_local({x, y, z});
}

template<typename T, precision Q>
inline void transform_t<T, Q>::rotate_local(const vec3_t& v) noexcept
{
    quat_t qx = glm::angleAxis(v.x, x_unit_axis());
    quat_t qy = glm::angleAxis(v.y, y_unit_axis());
    quat_t qz = glm::angleAxis(v.z, z_unit_axis());
    quat_t q = qz * qy * qx * get_rotation();
    set_rotation(q);
}

template<typename T, precision Q>
inline void transform_t<T, Q>::scale(T x, T y, T z) noexcept
{
    scale({x, y, z});
}

template<typename T, precision Q>
inline void transform_t<T, Q>::scale(const vec3_t& v) noexcept
{
    set_scale(get_scale() * v);
}

template<typename T, precision Q>
inline void transform_t<T, Q>::translate(T x, T y, T z) noexcept
{
    translate({x, y, z});
}

template<typename T, precision Q>
inline void transform_t<T, Q>::translate(const vec3_t& v) noexcept
{
    set_position(get_position() + v);
}

template<typename T, precision Q>
inline void transform_t<T, Q>::translate_local(T x, T y, T z) noexcept
{
    translate_local({x, y, z});
}

template<typename T, precision Q>
inline void transform_t<T, Q>::translate_local(const vec3_t& v) noexcept
{
    // Compute the local translation by using unit axes
    vec3_t local_translation = (x_unit_axis() * v.x) + (y_unit_axis() * v.y) + (z_unit_axis() * v.z);

    // Update the position once with the accumulated translation
    set_position(get_position() + local_translation);
}

template<typename T, precision Q>
inline auto transform_t<T, Q>::compare(const transform_t& t) const noexcept -> int
{
    return compare(t, epsilon<T>());
}

template<typename T, precision Q>
inline auto transform_t<T, Q>::compare(const transform_t& t, T tolerance) const noexcept -> int
{
    const auto& m1 = get_matrix();
    const auto& m2 = t.get_matrix();

    for(int i = 0; i < 4; ++i)
    {
        const vec4_t& row1 = m1[i];
        const vec4_t& row2 = m2[i];
        vec4_t diff = row1 - row2;

        for(int j = 0; j < 4; ++j)
        {
            if(glm::abs(diff[j]) > tolerance)
            {
                return (diff[j] < 0) ? -1 : 1;
            }
        }
    }

    // Equivalent
    return 0;
}

template<typename T, precision Q>
inline auto transform_t<T, Q>::transform_coord(const vec2_t& v) const noexcept -> typename transform_t<T, Q>::vec2_t
{
    const mat4_t& m = get_matrix();
    vec4_t result = m * vec4_t{v, 0.0f, 1.0f};
    result /= result.w;
    return result;
}

template<typename T, precision Q>
inline auto transform_t<T, Q>::inverse_transform_coord(const vec2_t& v) const noexcept ->
    typename transform_t<T, Q>::vec2_t
{
    const mat4_t& m = get_matrix();
    mat4_t im = glm::inverse(m);
    vec3_t result = im * vec4_t{v, 0.0f, 1.0f};
    return result;
}

template<typename T, precision Q>
inline auto transform_t<T, Q>::transform_normal(const vec2_t& v) const noexcept -> typename transform_t<T, Q>::vec2_t
{
    const mat4_t& m = get_matrix();
    vec4_t result = m * vec4_t{v, 0.0f, 0.0f};
    return result;
}

template<typename T, precision Q>
inline auto transform_t<T, Q>::inverse_transform_normal(const vec2_t& v) const noexcept ->
    typename transform_t<T, Q>::vec2_t
{
    const mat4_t& m = get_matrix();
    mat4_t im = glm::inverse(m);
    vec3_t result = im * vec4_t{v, 0.0f, 0.0f};
    return result;
}

template<typename T, precision Q>
inline auto transform_t<T, Q>::transform_coord(const vec3_t& v) const noexcept -> typename transform_t<T, Q>::vec3_t
{
    const mat4_t& m = get_matrix();
    vec4_t result = m * vec4_t{v, 1.0f};
    result /= result.w;
    return result;
}

template<typename T, precision Q>
inline auto transform_t<T, Q>::inverse_transform_coord(const vec3_t& v) const noexcept ->
    typename transform_t<T, Q>::vec3_t
{
    const mat4_t& m = get_matrix();
    mat4_t im = glm::inverse(m);
    vec3_t result = im * vec4_t{v, 1.0f};
    return result;
}

template<typename T, precision Q>
inline auto transform_t<T, Q>::transform_normal(const vec3_t& v) const noexcept -> typename transform_t<T, Q>::vec3_t
{
    const mat4_t& m = get_matrix();
    vec4_t result = m * vec4_t{v, 0.0f, 0.0f};
    return result;
}

template<typename T, precision Q>
inline auto transform_t<T, Q>::inverse_transform_normal(const vec3_t& v) const noexcept ->
    typename transform_t<T, Q>::vec3_t
{
    const mat4_t& m = get_matrix();
    mat4_t im = glm::inverse(m);
    vec3_t result = im * vec4_t{v, 0.0f};
    return result;
}

template<typename T, precision Q>
inline auto transform_t<T, Q>::identity() noexcept -> const transform_t<T, Q>&
{
    static transform_t identity;
    return identity;
}

template<typename T, precision Q>
inline auto transform_t<T, Q>::scaling(const vec2_t& scale) -> transform_t<T, Q>
{
    transform_t result{};
    result.set_scale(vec3_t(scale, 1.0f));
    return result;
}

template<typename T, precision Q>
inline auto transform_t<T, Q>::scaling(const vec3_t& scale) -> transform_t<T, Q>
{
    transform_t result{};
    result.set_scale(scale);
    return result;
}

template<typename T, precision Q>
inline auto transform_t<T, Q>::rotation(const quat_t& rotation) -> transform_t<T, Q>
{
    transform_t result{};
    result.set_rotation(rotation);
    return result;
}

template<typename T, precision Q>
inline auto transform_t<T, Q>::rotation_euler(const vec3_t& euler_angles) -> transform_t<T, Q>
{
    transform_t result{};
    result.set_rotation_euler(euler_angles);
    return result;
}

template<typename T, precision Q>
inline auto transform_t<T, Q>::translation(const vec2_t& trans) -> transform_t<T, Q>
{
    transform_t result{};
    result.set_position(vec3_t(trans, 0.0f));
    return result;
}

template<typename T, precision Q>
inline auto transform_t<T, Q>::translation(const vec3_t& trans) -> transform_t<T, Q>
{
    transform_t result{};
    result.set_position(trans);
    return result;
}

template<typename T, precision Q>
inline auto transform_t<T, Q>::operator*(const transform_t& t) const noexcept -> transform_t<T, Q>
{
    transform_t result(get_matrix() * t.get_matrix());
    return result;
}

template<typename T, precision Q>
inline auto transform_t<T, Q>::operator==(const transform_t& t) const noexcept -> bool
{
    return compare(t, math::epsilon<float>()) == 0;
}

template<typename T, precision Q>
inline auto transform_t<T, Q>::operator!=(const transform_t& t) const noexcept -> bool
{
    return compare(t, math::epsilon<float>()) != 0;
}

template<typename T, precision Q>
inline auto transform_t<T, Q>::get_matrix() const noexcept -> const mat4_t&
{
    update_matrix();
    return matrix_;
}

template<typename T, precision Q>
inline void transform_t<T, Q>::update_components() noexcept
{
    glm_decompose(matrix_, scale_, rotation_, position_, skew_, perspective_);
}

template<typename T, precision Q>
inline void transform_t<T, Q>::update_matrix() const noexcept
{
    if(dirty_)
    {
        glm_recompose(matrix_, scale_, rotation_, position_, skew_, perspective_);

        dirty_ = false;
    }
}

template<typename T, precision Q>
inline void transform_t<T, Q>::make_dirty() noexcept
{
    dirty_ = true;
}

template<typename T, precision Q>
inline auto transform_t<T, Q>::operator[](mat4_t::length_type i) const noexcept -> const typename mat4_t::col_type&
{
    return get_matrix()[i];
}

template<typename T, precision Q>
inline auto transform_t<T, Q>::operator*(const vec4_t& v) const noexcept -> vec4_t
{
    vec4_t result = get_matrix() * v;
    return result;
}

template<typename T, precision Q>
inline transform_t<T, Q>::operator const T*() const noexcept
{
    return value_ptr(get_matrix());
}

template<typename T, precision Q>
inline transform_t<T, Q>::operator const typename transform_t<T, Q>::mat4_t *() const noexcept
{
    return &get_matrix();
}

template<typename T, precision Q>
inline transform_t<T, Q>::operator const typename transform_t<T, Q>::mat4_t &() const noexcept
{
    return get_matrix();
}

using transform = transform_t<float>;

template<typename T>
inline std::string to_string(const T& v)
{
    return glm::to_string(v);
}
} // namespace math

namespace glm
{
namespace detail
{
template<typename T, qualifier Q>
struct compute_to_string<math::transform_t<T, Q>>
{
    GLM_FUNC_QUALIFIER static std::string call(math::transform_t<T, Q> const& x)
    {
        char const* PrefixStr = prefix<T>::value();
        return detail::format("%stransform((translation:%s, scale:%s, rotation:%s/rotation_euler:%s))",
                              PrefixStr,
                              to_string(x.get_position()).c_str(),
                              to_string(x.get_scale()).c_str(),
                              to_string(x.get_rotation()).c_str(),
                              to_string(x.get_rotation_euler()).c_str());
    }
};
} // namespace detail
} // namespace glm
