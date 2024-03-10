#pragma once
#include "quaternion.hpp"
#include "vector.hpp"

namespace cereal
{
template<typename Archive, typename T, math::qualifier P>
inline void SERIALIZE_FUNCTION_NAME(Archive& ar, math::tmat2x2<T, P>& obj)
{
    try_serialize(ar, cereal::make_nvp("col_0", obj[0]));
    try_serialize(ar, cereal::make_nvp("col_1", obj[1]));
}

template<typename Archive, typename T, math::qualifier P>
inline void SERIALIZE_FUNCTION_NAME(Archive& ar, math::tmat2x3<T, P>& obj)
{
    try_serialize(ar, cereal::make_nvp("col_0", obj[0]));
    try_serialize(ar, cereal::make_nvp("col_1", obj[1]));
    try_serialize(ar, cereal::make_nvp("col_2", obj[2]));
}
template<typename Archive, typename T, math::qualifier P>
inline void SERIALIZE_FUNCTION_NAME(Archive& ar, math::tmat2x4<T, P>& obj)
{
    try_serialize(ar, cereal::make_nvp("col_0", obj[0]));
    try_serialize(ar, cereal::make_nvp("col_1", obj[1]));
    try_serialize(ar, cereal::make_nvp("col_2", obj[2]));
    try_serialize(ar, cereal::make_nvp("col_3", obj[3]));
}

template<typename Archive, typename T, math::qualifier P>
inline void SERIALIZE_FUNCTION_NAME(Archive& ar, math::tmat3x2<T, P>& obj)
{
    try_serialize(ar, cereal::make_nvp("col_0", obj[0]));
    try_serialize(ar, cereal::make_nvp("col_1", obj[1]));
}

template<typename Archive, typename T, math::qualifier P>
inline void SERIALIZE_FUNCTION_NAME(Archive& ar, math::tmat4x2<T, P>& obj)
{
    try_serialize(ar, cereal::make_nvp("col_0", obj[0]));
    try_serialize(ar, cereal::make_nvp("col_1", obj[1]));
}

template<typename Archive, typename T, math::qualifier P>
inline void SERIALIZE_FUNCTION_NAME(Archive& ar, math::tmat3x3<T, P>& obj)
{
    try_serialize(ar, cereal::make_nvp("col_0", obj[0]));
    try_serialize(ar, cereal::make_nvp("col_1", obj[1]));
    try_serialize(ar, cereal::make_nvp("col_2", obj[2]));
}

template<typename Archive, typename T, math::qualifier P>
inline void SERIALIZE_FUNCTION_NAME(Archive& ar, math::tmat3x4<T, P>& obj)
{
    try_serialize(ar, cereal::make_nvp("col_0", obj[0]));
    try_serialize(ar, cereal::make_nvp("col_1", obj[1]));
    try_serialize(ar, cereal::make_nvp("col_2", obj[2]));
    try_serialize(ar, cereal::make_nvp("col_3", obj[3]));
}

template<typename Archive, typename T, math::qualifier P>
inline void SERIALIZE_FUNCTION_NAME(Archive& ar, math::tmat4x3<T, P>& obj)
{
    try_serialize(ar, cereal::make_nvp("col_0", obj[0]));
    try_serialize(ar, cereal::make_nvp("col_1", obj[1]));
    try_serialize(ar, cereal::make_nvp("col_2", obj[2]));
}

template<typename Archive, typename T, math::qualifier P>
inline void SERIALIZE_FUNCTION_NAME(Archive& ar, math::tmat4x4<T, P>& obj)
{
    try_serialize(ar, cereal::make_nvp("col_0", obj[0]));
    try_serialize(ar, cereal::make_nvp("col_1", obj[1]));
    try_serialize(ar, cereal::make_nvp("col_2", obj[2]));
    try_serialize(ar, cereal::make_nvp("col_3", obj[3]));
}

template<typename Archive, typename T, math::qualifier P>
inline void SERIALIZE_FUNCTION_NAME(Archive& ar, math::transform_t<T, P>& obj)
{
    auto pos = obj.get_position();
    auto rot = obj.get_rotation();
    auto scale = obj.get_scale();
    auto skew = obj.get_skew();

    try_serialize(ar, cereal::make_nvp("position", pos));
    try_serialize(ar, cereal::make_nvp("rotation", rot));
    try_serialize(ar, cereal::make_nvp("scale", scale));
    try_serialize(ar, cereal::make_nvp("skew", skew));

    obj.set_position(pos);
    obj.set_rotation(rot);
    obj.set_scale(scale);
    obj.set_skew(skew);
}
} // namespace cereal
