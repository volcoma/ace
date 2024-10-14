#include "script.hpp"

#include <fstream>
#include <serialization/associative_archive.h>
#include <serialization/binary_archive.h>

namespace ace
{

REFLECT(script)
{
    rttr::registration::class_<script>("script")(rttr::metadata("pretty_name", "Script"))
    .constructor<>()();
}

SAVE(script)
{

}
SAVE_INSTANTIATE(script, ser20::oarchive_associative_t);
SAVE_INSTANTIATE(script, ser20::oarchive_binary_t);

LOAD(script)
{

}
LOAD_INSTANTIATE(script, ser20::iarchive_associative_t);
LOAD_INSTANTIATE(script, ser20::iarchive_binary_t);

void save_to_file(const std::string& absolute_path, const script::sptr& obj)
{
    std::ofstream stream(absolute_path);
    if(stream.good())
    {
        auto ar = ser20::create_oarchive_associative(stream);
        try_save(ar, ser20::make_nvp("script", *obj));
    }
}

void save_to_file_bin(const std::string& absolute_path, const script::sptr& obj)
{
    std::ofstream stream(absolute_path, std::ios::binary);
    if(stream.good())
    {
        ser20::oarchive_binary_t ar(stream);
        try_save(ar, ser20::make_nvp("script", *obj));
    }
}

void load_from_file(const std::string& absolute_path, script::sptr& obj)
{
    std::ifstream stream(absolute_path);
    if(stream.good())
    {
        auto ar = ser20::create_iarchive_associative(stream);
        try_load(ar, ser20::make_nvp("script", *obj));
    }
}

void load_from_file_bin(const std::string& absolute_path, script::sptr& obj)
{
    std::ifstream stream(absolute_path, std::ios::binary);
    if(stream.good())
    {
        ser20::iarchive_binary_t ar(stream);
        try_load(ar, ser20::make_nvp("script", *obj));
    }
}
} // namespace ace
