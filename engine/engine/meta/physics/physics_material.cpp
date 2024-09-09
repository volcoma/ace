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
        .property("restitution", &physics_material::restitution)(
            rttr::metadata("pretty_name", "Restitution"),
            rttr::metadata(
                "tooltip",
                "Restitution represents the bounciness of the material. A value of 0.0 means no bounce (perfectly "
                "inelastic collision), while 1.0 means perfect bounce (perfectly elastic collision)."),
            rttr::metadata("min", 0.0f),
            rttr::metadata("max", 1.0f))
        .property("friction", &physics_material::friction)(
            rttr::metadata("pretty_name", "Friction"),
            rttr::metadata(
                "tooltip",
                "Friction represents the resistance to sliding motion. A value of 0.0 means no friction (perfectly "
                "slippery), while values around 1.0 represent typical real-world friction. Values slightly above 1.0 "
                "can simulate very high friction surfaces but should be used cautiously."),
            rttr::metadata("min", 0.0f),
            rttr::metadata("max", 1.0f))
        .property("spin_friction", &physics_material::spin_friction)(
            rttr::metadata("pretty_name", "Spin Friction"),
            rttr::metadata("tooltip",
                           "Spin friction (or torsional friction) represents resistance to rotational motion around "
                           "the contact normal. Similar to regular friction, 0.0 means no spin friction, while values "
                           "around 1.0 represent typical high friction."),
            rttr::metadata("min", 0.0f),
            rttr::metadata("max", 1.0f))
        .property("roll_friction", &physics_material::roll_friction)(
            rttr::metadata("pretty_name", "Roll Friction"),
            rttr::metadata("tooltip",
                           "Roll friction represents resistance to rolling motion. Like other friction values, 0.0 "
                           "means no resistance to rolling, while values around 1.0 simulate high rolling resistance."),
            rttr::metadata("min", 0.0f),
            rttr::metadata("max", 1.0f))
        .property("stiffness", &physics_material::stiffness)(
            rttr::metadata("pretty_name", "Stiffness"),
            rttr::metadata("tooltip",
                           "Stiffness represents how much force is required to deform the material. A high value means "
                           "the material is very stiff (resists deformation)."),
            rttr::metadata("min", 0.0f),
            rttr::metadata("max", 1.0f))
        .property("damping", &physics_material::damping)(
            rttr::metadata("pretty_name", "Damping"),
            rttr::metadata("tooltip",
                           "Damping represents energy loss in motion (e.g., through internal friction). A value of 0.0 "
                           "means no damping (energy is conserved), while 1.0 represents very high damping (rapid "
                           "energy loss). Typical values range from 0.01 to 0.3 for realistic simulations."),
            rttr::metadata("min", 0.0f),
            rttr::metadata("max", 1.0f));
}

SAVE(physics_material)
{
    try_save(ar, ser20::make_nvp("restitution", obj.restitution));
    try_save(ar, ser20::make_nvp("friction", obj.friction));
    try_save(ar, ser20::make_nvp("spin_friction", obj.spin_friction));
    try_save(ar, ser20::make_nvp("roll_friction", obj.roll_friction));
    try_save(ar, ser20::make_nvp("stiffness", obj.stiffness));
    try_save(ar, ser20::make_nvp("damping", obj.damping));
}
SAVE_INSTANTIATE(physics_material, ser20::oarchive_associative_t);
SAVE_INSTANTIATE(physics_material, ser20::oarchive_binary_t);

LOAD(physics_material)
{
    try_load(ar, ser20::make_nvp("restitution", obj.restitution));
    try_load(ar, ser20::make_nvp("friction", obj.friction));
    try_load(ar, ser20::make_nvp("spin_friction", obj.spin_friction));
    try_load(ar, ser20::make_nvp("roll_friction", obj.roll_friction));
    try_load(ar, ser20::make_nvp("stiffness", obj.stiffness));
    try_load(ar, ser20::make_nvp("damping", obj.damping));
}
LOAD_INSTANTIATE(physics_material, ser20::iarchive_associative_t);
LOAD_INSTANTIATE(physics_material, ser20::iarchive_binary_t);

void save_to_file(const std::string& absolute_path, const physics_material::sptr& obj)
{
    std::ofstream stream(absolute_path);
    if(stream.good())
    {
        ser20::oarchive_associative_t ar(stream);
        try_save(ar, ser20::make_nvp("physics_material", *obj));
    }
}

void save_to_file_bin(const std::string& absolute_path, const physics_material::sptr& obj)
{
    std::ofstream stream(absolute_path, std::ios::binary);
    if(stream.good())
    {
        ser20::oarchive_binary_t ar(stream);
        try_save(ar, ser20::make_nvp("physics_material", *obj));
    }
}

void load_from_file(const std::string& absolute_path, physics_material::sptr& obj)
{
    std::ifstream stream(absolute_path);
    if(stream.good())
    {
        ser20::iarchive_associative_t ar(stream);
        try_load(ar, ser20::make_nvp("physics_material", *obj));
    }
}

void load_from_file_bin(const std::string& absolute_path, physics_material::sptr& obj)
{
    std::ifstream stream(absolute_path, std::ios::binary);
    if(stream.good())
    {
        ser20::iarchive_binary_t ar(stream);
        try_load(ar, ser20::make_nvp("physics_material", *obj));
    }
}
} // namespace ace
