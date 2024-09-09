#include "material.hpp"

#include <fstream>
#include <serialization/associative_archive.h>
#include <serialization/binary_archive.h>

namespace ace
{

REFLECT(material)
{
    rttr::registration::enumeration<cull_type>("cull_type")(
        rttr::value("None", cull_type::none),
        rttr::value("Clockwise", cull_type::clockwise),
        rttr::value("Counter Clockwise", cull_type::counter_clockwise));

    rttr::registration::class_<material>("material")
        .property("cull_type", &material::get_cull_type, &material::set_cull_type)(
            rttr::metadata("pretty_name", "Cull Type"));
}

SAVE(material)
{
    try_save(ar, ser20::make_nvp("cull_type", obj.cull_type_));
}
SAVE_INSTANTIATE(material, ser20::oarchive_associative_t);
SAVE_INSTANTIATE(material, ser20::oarchive_binary_t);

LOAD(material)
{
    try_load(ar, ser20::make_nvp("cull_type", obj.cull_type_));
}
LOAD_INSTANTIATE(material, ser20::iarchive_associative_t);
LOAD_INSTANTIATE(material, ser20::iarchive_binary_t);

void save_to_file(const std::string& absolute_path, const std::shared_ptr<material>& obj)
{
    std::ofstream stream(absolute_path);
    if(stream.good())
    {
        ser20::oarchive_associative_t ar(stream);
        try_save(ar, ser20::make_nvp("material", obj));
    }
}

void save_to_file_bin(const std::string& absolute_path, const std::shared_ptr<material>& obj)
{
    std::ofstream stream(absolute_path, std::ios::binary);
    if(stream.good())
    {
        ser20::oarchive_binary_t ar(stream);
        try_save(ar, ser20::make_nvp("material", obj));
    }
}

void load_from_file(const std::string& absolute_path, std::shared_ptr<material>& obj)
{
    std::ifstream stream(absolute_path);
    if(stream.good())
    {
        ser20::iarchive_associative_t ar(stream);
        try_load(ar, ser20::make_nvp("material", obj));
    }
}

void load_from_file_bin(const std::string& absolute_path, std::shared_ptr<material>& obj)
{
    std::ifstream stream(absolute_path, std::ios::binary);
    if(stream.good())
    {
        ser20::iarchive_binary_t ar(stream);
        try_load(ar, ser20::make_nvp("material", obj));
    }
}

} // namespace ace
