#include "animation.hpp"
#include <engine/meta/core/math/quaternion.hpp>
#include <engine/meta/core/math/transform.hpp>

#include <fstream>
#include <serialization/associative_archive.h>
#include <serialization/binary_archive.h>

namespace ace
{
REFLECT(animation_channel)
{
    rttr::registration::class_<animation_channel>("animation_channel")
        .property_readonly("node_name", &animation_channel::node_name)(rttr::metadata("pretty_name", "Name"));
}

REFLECT(animation_clip)
{
    rttr::registration::class_<animation_clip>("animation")
        .property_readonly("name", &animation_clip::name)(rttr::metadata("pretty_name", "Name"))
        .property_readonly("duration", &animation_clip::duration)(rttr::metadata("pretty_name", "Duration"))
        .property_readonly("channels", &animation_clip::channels)(rttr::metadata("pretty_name", "Channels"));
}

SAVE(animation_channel)
{
    try_save(ar, ser20::make_nvp("node_name", obj.node_name));
    try_save(ar, ser20::make_nvp("node_index", obj.node_index));
    try_save(ar, ser20::make_nvp("position_keys", obj.position_keys));
    try_save(ar, ser20::make_nvp("rotation_keys", obj.rotation_keys));
    try_save(ar, ser20::make_nvp("scaling_keys", obj.scaling_keys));
}
SAVE_INSTANTIATE(animation_channel, ser20::oarchive_associative_t);
SAVE_INSTANTIATE(animation_channel, ser20::oarchive_binary_t);

LOAD(animation_channel)
{
    try_load(ar, ser20::make_nvp("node_name", obj.node_name));
    try_load(ar, ser20::make_nvp("node_index", obj.node_index));
    try_load(ar, ser20::make_nvp("position_keys", obj.position_keys));
    try_load(ar, ser20::make_nvp("rotation_keys", obj.rotation_keys));
    try_load(ar, ser20::make_nvp("scaling_keys", obj.scaling_keys));
}
LOAD_INSTANTIATE(animation_channel, ser20::iarchive_associative_t);
LOAD_INSTANTIATE(animation_channel, ser20::iarchive_binary_t);

SAVE(animation_clip)
{
    try_save(ar, ser20::make_nvp("name", obj.name));
    try_save(ar, ser20::make_nvp("duration", obj.duration));
    try_save(ar, ser20::make_nvp("channels", obj.channels));
}
SAVE_INSTANTIATE(animation_clip, ser20::oarchive_associative_t);
SAVE_INSTANTIATE(animation_clip, ser20::oarchive_binary_t);

LOAD(animation_clip)
{
    try_load(ar, ser20::make_nvp("name", obj.name));
    try_load(ar, ser20::make_nvp("duration", obj.duration));
    try_load(ar, ser20::make_nvp("channels", obj.channels));
}
LOAD_INSTANTIATE(animation_clip, ser20::iarchive_associative_t);
LOAD_INSTANTIATE(animation_clip, ser20::iarchive_binary_t);

void save_to_file(const std::string& absolute_path, const animation_clip& obj)
{
    std::ofstream stream(absolute_path);
    if(stream.good())
    {
        auto ar = ser20::create_oarchive_associative(stream);
        try_save(ar, ser20::make_nvp("animation", obj));
    }
}

void save_to_file_bin(const std::string& absolute_path, const animation_clip& obj)
{
    std::ofstream stream(absolute_path, std::ios::binary);
    if(stream.good())
    {
        ser20::oarchive_binary_t ar(stream);
        try_save(ar, ser20::make_nvp("animation", obj));
    }
}

void load_from_file(const std::string& absolute_path, animation_clip& obj)
{
    std::ifstream stream(absolute_path);
    if(stream.good())
    {
        auto ar = ser20::create_iarchive_associative(stream);
        try_load(ar, ser20::make_nvp("animation", obj));
    }
}

void load_from_file_bin(const std::string& absolute_path, animation_clip& obj)
{
    std::ifstream stream(absolute_path, std::ios::binary);
    if(stream.good())
    {
        ser20::iarchive_binary_t ar(stream);
        try_load(ar, ser20::make_nvp("animation", obj));
    }
}
} // namespace ace
