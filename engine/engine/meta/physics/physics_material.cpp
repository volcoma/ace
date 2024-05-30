#include "physics_material.hpp"

#include <fstream>
#include <serialization/associative_archive.h>
#include <serialization/binary_archive.h>

namespace ace
{

REFLECT(physics_material)
{
    rttr::registration::class_<physics_material>("physics_material")(rttr::metadata("pretty_name", "Physics Material"))
        .constructor<>()()
        .property("restitution", &physics_material::restitution)(rttr::metadata("pretty_name", "Restitution"),
                                                                 rttr::metadata("tooltip", "Missing..."))
        .property("friction", &physics_material::friction)(rttr::metadata("pretty_name", "Friction"),
                                                           rttr::metadata("tooltip", "Missing..."))
        .property("spin_friction", &physics_material::spin_friction)(rttr::metadata("pretty_name", "Spin Friction"),
                                                                     rttr::metadata("tooltip", "Missing..."))
        .property("roll_friction", &physics_material::roll_friction)(rttr::metadata("pretty_name", "Roll Friction"),
                                                                     rttr::metadata("tooltip", "Missing..."))
        .property("stiffness", &physics_material::stiffness)(rttr::metadata("pretty_name", "Stiffness"),
                                                             rttr::metadata("tooltip", "Missing..."))
        .property("damping", &physics_material::damping)(rttr::metadata("pretty_name", "Damping"),
                                                         rttr::metadata("tooltip", "Missing..."));
}

SAVE(physics_material)
{
    try_save(ar, cereal::make_nvp("restitution", obj.restitution));
    try_save(ar, cereal::make_nvp("friction", obj.friction));
    try_save(ar, cereal::make_nvp("spin_friction", obj.spin_friction));
    try_save(ar, cereal::make_nvp("roll_friction", obj.roll_friction));
    try_save(ar, cereal::make_nvp("stiffness", obj.stiffness));
    try_save(ar, cereal::make_nvp("damping", obj.damping));
}
SAVE_INSTANTIATE(physics_material, cereal::oarchive_associative_t);
SAVE_INSTANTIATE(physics_material, cereal::oarchive_binary_t);

LOAD(physics_material)
{
    try_load(ar, cereal::make_nvp("restitution", obj.restitution));
    try_load(ar, cereal::make_nvp("friction", obj.friction));
    try_load(ar, cereal::make_nvp("spin_friction", obj.spin_friction));
    try_load(ar, cereal::make_nvp("roll_friction", obj.roll_friction));
    try_load(ar, cereal::make_nvp("stiffness", obj.stiffness));
    try_load(ar, cereal::make_nvp("damping", obj.damping));
}
LOAD_INSTANTIATE(physics_material, cereal::iarchive_associative_t);
LOAD_INSTANTIATE(physics_material, cereal::iarchive_binary_t);

void save_to_file(const std::string& absolute_path, const physics_material::sptr& obj)
{
    std::ofstream stream(absolute_path);
    if(stream.good())
    {
        cereal::oarchive_associative_t ar(stream);
        try_save(ar, cereal::make_nvp("physics_material", *obj));
    }
}

void save_to_file_bin(const std::string& absolute_path, const physics_material::sptr& obj)
{
    std::ofstream stream(absolute_path, std::ios::binary);
    if(stream.good())
    {
        cereal::oarchive_binary_t ar(stream);
        try_save(ar, cereal::make_nvp("physics_material", *obj));
    }
}

void load_from_file(const std::string& absolute_path, physics_material::sptr& obj)
{
    std::ifstream stream(absolute_path);
    if(stream.good())
    {
        cereal::iarchive_associative_t ar(stream);
        try_load(ar, cereal::make_nvp("physics_material", *obj));
    }
}

void load_from_file_bin(const std::string& absolute_path, physics_material::sptr& obj)
{
    std::ifstream stream(absolute_path, std::ios::binary);
    if(stream.good())
    {
        cereal::iarchive_binary_t ar(stream);
        try_load(ar, cereal::make_nvp("physics_material", *obj));
    }
}
} // namespace ace
