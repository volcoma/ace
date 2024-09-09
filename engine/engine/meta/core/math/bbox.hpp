#pragma once
#include "vector.hpp"

namespace ser20
{
template<typename Archive>
inline void SERIALIZE_FUNCTION_NAME(Archive& ar, math::bbox& obj)
{
    try_serialize(ar, ser20::make_nvp("min", obj.min));
    try_serialize(ar, ser20::make_nvp("min", obj.max));
}
} // namespace ser20
