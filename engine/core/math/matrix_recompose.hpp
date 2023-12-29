#pragma once

#include "detail/glm_includes.h"

namespace math
{
using namespace glm;




template<typename T, precision Q>
inline void glm_recompose(mat<4, 4, T, Q> & model_matrix,
                          vec<3, T, Q> const& in_scale,
                          qua<T, Q> const& in_orientation,
                          vec<3, T, Q> const& in_translation,
                          vec<3, T, Q> const& in_skew_xz_yz_xy,
                          vec<4, T, Q> const& in_perspective)
{

    auto in_sscale = in_scale;
    for(math::length_t i = 0; i < in_sscale.length(); ++i)
    {
        auto& el = in_sscale[i];
        if(math::epsilonEqual<T>(el, T(0), epsilon<T>()))
        {
            el = T(0.0001);
        }
    }

    model_matrix = recompose(in_sscale, in_orientation, in_translation, in_skew_xz_yz_xy, in_perspective);
}


// Recomposes a model matrix from a previously-decomposed matrix
// http://www.opensource.apple.com/source/WebCore/WebCore-514/platform/graphics/transforms/TransformationMatrix.cpp
// https://stackoverflow.com/a/75573092/1047040
// template <typename T, qualifier Q>
// inline mat<4, 4, T, Q> recompose(
//     vec<3, T, Q> const& scale, qua<T, Q> const& orientation, vec<3, T, Q> const& translation,
//     vec<3, T, Q> const& skew, vec<4, T, Q> const& perspective)
// {
//     glm::mat4 m = glm::mat4(1.f);

//     m[0][3] = perspective.x;
//     m[1][3] = perspective.y;
//     m[2][3] = perspective.z;
//     m[3][3] = perspective.w;

//     m *= glm::translate(translation);
//     m *= glm::mat4_cast(orientation);

//     if (abs(skew.x) > static_cast<T>(0)) {
//         glm::mat4 tmp(1.f);
//         tmp[2][1] = skew.x;
//         m *= tmp;
//     }

//     if (abs(skew.y) > static_cast<T>(0)) {
//         glm::mat4 tmp(1.f);
//         tmp[2][0] = skew.y;
//         m *= tmp;
//     }

//     if (abs(skew.z) > static_cast<T>(0)) {
//         glm::mat4 tmp(1.f);
//         tmp[1][0] = skew.z;
//         m *= tmp;
//     }

//     m *= glm::scale(scale);

//     return m;
// }

//Input: matrix       ; a 4x4 matrix
//Output: translation ; a 3 component vector
//rotation    ; a quaternion
//scale       ; a 3 component vector
//skew        ; skew factors XZ,YZ,XY, represented as a 3 component vector
//perspective ; a 4 component vector
//template<typename T, precision Q>
//inline bool glm_decompose(mat<4, 4, T, Q> const& model_matrix,
//                          vec<3, T, Q> & scale,
//                          qua<T, Q> & orientation,
//                          vec<3, T, Q> & translation,
//                          vec<3, T, Q> & skew,
//                          vec<4, T, Q> & perspective)
//{
//    mat<4, 4, T, Q> local_matrix(model_matrix);

//    // Normalize the matrix.
//    if(epsilonEqual(local_matrix[3][3], static_cast<T>(0), epsilon<T>()))
//        return false;

//    for(length_t i = 0; i < 4; ++i)
//    for(length_t j = 0; j < 4; ++j)
//        local_matrix[i][j] /= local_matrix[3][3];

//    // perspectiveMatrix is used to solve for perspective, but it also provides
//    // an easy way to test for singularity of the upper 3x3 component.
//    mat<4, 4, T, Q> perspective_matrix(local_matrix);

//    for(length_t i = 0; i < 3; i++)
//        perspective_matrix[i][3] = static_cast<T>(0);
//    perspective_matrix[3][3] = T(1);

//    if(epsilonEqual(determinant(perspective_matrix), static_cast<T>(0), epsilon<T>()))
//        return false;

//    // First, isolate perspective.  This is the messiest.
//    if(
//        epsilonNotEqual(local_matrix[0][3], static_cast<T>(0), epsilon<T>()) ||
//        epsilonNotEqual(local_matrix[1][3], static_cast<T>(0), epsilon<T>()) ||
//        epsilonNotEqual(local_matrix[2][3], static_cast<T>(0), epsilon<T>()))
//    {
//        // rightHandSide is the right hand side of the equation.
//        vec<4, T, Q> right_hand_side;
//        right_hand_side[0] = local_matrix[0][3];
//        right_hand_side[1] = local_matrix[1][3];
//        right_hand_side[2] = local_matrix[2][3];
//        right_hand_side[3] = local_matrix[3][3];

//        // Solve the equation by inverting PerspectiveMatrix and multiplying
//        // rightHandSide by the inverse.  (This is the easiest way, not
//        // necessarily the best.)
//        mat<4, 4, T, Q> inverse_perspective_matrix = glm::inverse(perspective_matrix);//   inverse(PerspectiveMatrix, inversePerspectiveMatrix);
//        mat<4, 4, T, Q> transposed_inverse_perspective_matrix = glm::transpose(inverse_perspective_matrix);//   transposeMatrix4(inversePerspectiveMatrix, transposedInversePerspectiveMatrix);

//        perspective = transposed_inverse_perspective_matrix * right_hand_side;
//        //  v4MulPointByMatrix(rightHandSide, transposedInversePerspectiveMatrix, perspectivePoint);

//        // Clear the perspective partition
//        local_matrix[0][3] = local_matrix[1][3] = local_matrix[2][3] = static_cast<T>(0);
//        local_matrix[3][3] = T(1);
//    }
//    else
//    {
//        // No perspective.
//        perspective = vec<4, T, Q>(0, 0, 0, 1);
//    }

//    // Next take care of translation (easy).
//    translation = vec<3, T, Q>(local_matrix[3]);
//    local_matrix[3] = vec<4, T, Q>(0, 0, 0, local_matrix[3].w);

//    vec<3, T, Q> row[3], pdum3;

//    // Now get scale and shear.
//    for(length_t i = 0; i < 3; ++i)
//    for(length_t j = 0; j < 3; ++j)
//        row[i][j] = local_matrix[i][j];

//    // Compute X scale factor and normalize first row.
//    scale.x = length(row[0]);// v3Length(Row[0]);

//    row[0] = detail::scale(row[0], T(1));

//    // Compute XY shear factor and make 2nd row orthogonal to 1st.
//    skew.z = dot(row[0], row[1]);
//    row[1] = detail::combine(row[1], row[0], T(1), -skew.z);

//    // Now, compute Y scale and normalize 2nd row.
//    scale.y = length(row[1]);
//    row[1] = detail::scale(row[1], T(1));
//    skew.z /= scale.y;

//    // Compute XZ and YZ shears, orthogonalize 3rd row.
//    skew.y = dot(row[0], row[2]);
//    row[2] = detail::combine(row[2], row[0], T(1), -skew.y);
//    skew.x = dot(row[1], row[2]);
//    row[2] = detail::combine(row[2], row[1], T(1), -skew.x);

//    // Next, get Z scale and normalize 3rd row.
//    scale.z = length(row[2]);
//    row[2] = detail::scale(row[2], T(1));
//    skew.y /= scale.z;
//    skew.x /= scale.z;

//    // At this point, the matrix (in rows[]) is orthonormal.
//    // Check for a coordinate system flip.  If the determinant
//    // is -1, then negate the matrix and the scaling factors.
//    pdum3 = cross(row[1], row[2]); // v3Cross(row[1], row[2], Pdum3);
//    if(dot(row[0], pdum3) < 0)
//    {
//        for(length_t i = 0; i < 3; i++)
//        {
//            scale[i] *= T(-1);
//            row[i] *= T(-1);
//        }
//    }

//    // Now, get the rotations out, as described in the gem.

//    // FIXME - Add the ability to return either quaternions (which are
//    // easier to recompose with) or Euler angles (rx, ry, rz), which
//    // are easier for authors to deal with. The latter will only be useful
//    // when we fix https://bugs.webkit.org/show_bug.cgi?id=23799, so I
//    // will leave the Euler angle code here for now.

//    // ret.rotateY = asin(-Row[0][2]);
//    // if (cos(ret.rotateY) != 0) {
//    //     ret.rotateX = atan2(Row[1][2], Row[2][2]);
//    //     ret.rotateZ = atan2(Row[0][1], Row[0][0]);
//    // } else {
//    //     ret.rotateX = atan2(-Row[2][0], Row[1][1]);
//    //     ret.rotateZ = 0;
//    // }

//    int i, j, k = 0;
//    float root, trace = row[0].x + row[1].y + row[2].z;
//    if(trace > static_cast<T>(0))
//    {
//        root = sqrt(trace + T(1.0));
//        orientation.w = static_cast<T>(0.5) * root;
//        root = static_cast<T>(0.5) / root;
//        orientation.x = root * (row[1].z - row[2].y);
//        orientation.y = root * (row[2].x - row[0].z);
//        orientation.z = root * (row[0].y - row[1].x);
//    } // End if > 0
//    else
//    {
//        static int Next[3] = {1, 2, 0};
//        i = 0;
//        if(row[1].y > row[0].x) i = 1;
//        if(row[2].z > row[i][i]) i = 2;
//        j = Next[i];
//        k = Next[j];

//        root = sqrt(row[i][i] - row[j][j] - row[k][k] + T(1.0));

//        orientation[i] = static_cast<T>(0.5) * root;
//        root = static_cast<T>(0.5) / root;
//        orientation[j] = root * (row[i][j] + row[j][i]);
//        orientation[k] = root * (row[i][k] + row[k][i]);
//        orientation.w = root * (row[j][k] - row[k][j]);
//    } // End if <= 0

//    return true;
//}

}
