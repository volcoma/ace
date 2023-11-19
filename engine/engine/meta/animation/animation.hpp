#pragma once

#include <engine/animation/animation.h>

#include <reflection/reflection.h>
#include <serialization/serialization.h>
#include <serialization/types/chrono.hpp>

namespace ace
{
SAVE_EXTERN(animation);
LOAD_EXTERN(animation);

SAVE_EXTERN(node_animation);
LOAD_EXTERN(node_animation);

template <typename Archive, typename T>
inline void SERIALIZE_FUNCTION_NAME(Archive& ar, node_animation::key<T>& obj)
{
	try_serialize(ar, cereal::make_nvp("time", obj.time));
	try_serialize(ar, cereal::make_nvp("value", obj.value));
}

REFLECT_EXTERN(node_animation);
REFLECT_EXTERN(animation);

void save_to_file(const std::string& absolute_path, const animation& obj);
void save_to_file_bin(const std::string& absolute_path, const animation& obj);
void load_from_file(const std::string& absolute_path, animation& obj);
void load_from_file_bin(const std::string& absolute_path, animation& obj);

}
