#pragma once

#include <uuid/uuid.h>
#include <base/basetypes.hpp>
#include <serialization/serialization.h>

namespace cereal
{
template<typename Archive, typename T>
inline void SERIALIZE_FUNCTION_NAME(Archive& ar, rect<T>& obj)
{
    try_serialize(ar, cereal::make_nvp("left", obj.left));
    try_serialize(ar, cereal::make_nvp("top", obj.top));
    try_serialize(ar, cereal::make_nvp("right", obj.right));
    try_serialize(ar, cereal::make_nvp("bottom", obj.bottom));
}

template<typename Archive, typename T>
inline void SERIALIZE_FUNCTION_NAME(Archive& ar, size<T>& obj)
{
    try_serialize(ar, cereal::make_nvp("width", obj.width));
    try_serialize(ar, cereal::make_nvp("height", obj.height));
}

template<typename Archive, typename T>
inline void SERIALIZE_FUNCTION_NAME(Archive& ar, point<T>& obj)
{
    try_serialize(ar, cereal::make_nvp("x", obj.x));
    try_serialize(ar, cereal::make_nvp("y", obj.y));
}

template<typename Archive, typename T>
inline void SERIALIZE_FUNCTION_NAME(Archive& ar, range<T>& obj)
{
    try_serialize(ar, cereal::make_nvp("min", obj.min));
    try_serialize(ar, cereal::make_nvp("max", obj.max));
}


template<typename Archive>
inline auto SAVE_MINIMAL_FUNCTION_NAME(Archive& ar, const hpp::uuid& obj) -> std::string
{
    return hpp::to_string(obj);
    //try_save(ar, cereal::make_nvp("data", hpp::to_string(obj)));
}

template<typename Archive>
inline void LOAD_MINIMAL_FUNCTION_NAME(Archive& ar, hpp::uuid& obj, const std::string& suuid)
{
    //std::string suuid;

    //try_load(ar, cereal::make_nvp("data", suuid));

    auto id = hpp::uuid::from_string(suuid);
    obj = id.value_or(hpp::uuid{});
}
} // namespace cereal
