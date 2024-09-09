#pragma once
#include <engine/engine_export.h>

#include <engine/animation/animation.h>

#include <reflection/reflection.h>
#include <serialization/serialization.h>
#include <serialization/types/chrono.hpp>

namespace ace
{
SAVE_EXTERN(animation_clip);
LOAD_EXTERN(animation_clip);

SAVE_EXTERN(animation_channel);
LOAD_EXTERN(animation_channel);

template<typename Archive, typename T>
void SERIALIZE_FUNCTION_NAME(Archive& ar, animation_channel::key<T>& obj)
{
    try_serialize(ar, ser20::make_nvp("time", obj.time));
    try_serialize(ar, ser20::make_nvp("value", obj.value));
}

void save_to_file(const std::string& absolute_path, const animation_clip& obj);
void save_to_file_bin(const std::string& absolute_path, const animation_clip& obj);
void load_from_file(const std::string& absolute_path, animation_clip& obj);
void load_from_file_bin(const std::string& absolute_path, animation_clip& obj);

} // namespace ace
