#pragma once

#include <math/math.h>
#include <serialization/serialization.h>

namespace ser20
{
template<typename Archive, typename T, math::qualifier P>
inline void SERIALIZE_FUNCTION_NAME(Archive& ar, math::tquat<T, P>& obj)
{
    try_serialize(ar, ser20::make_nvp("x", obj.x));
    try_serialize(ar, ser20::make_nvp("y", obj.y));
    try_serialize(ar, ser20::make_nvp("z", obj.z));
    try_serialize(ar, ser20::make_nvp("w", obj.w));
}
} // namespace ser20
