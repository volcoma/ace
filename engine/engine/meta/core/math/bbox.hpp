#pragma once
#include "vector.hpp"

namespace cereal
{
template<typename Archive>
inline void SERIALIZE_FUNCTION_NAME(Archive& ar, math::bbox& obj)
{
    try_serialize(ar, cereal::make_nvp("min", obj.min));
    try_serialize(ar, cereal::make_nvp("min", obj.max));
}
} // namespace cereal
