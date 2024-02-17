#include "rigidbody_component.hpp"
#include "engine/ecs/components/rigidbody_component.h"

#include <serialization/associative_archive.h>
#include <serialization/binary_archive.h>

namespace ace
{
REFLECT(rigidbody_component)
{
    rttr::registration::class_<rigidbody_component>("rigidbody_component")(rttr::metadata("category", "PHYSICS"),
                                                                           rttr::metadata("pretty_name", "Rigidbody"))
        .constructor<>()()
        .property("is_using_gravity",
                  &rigidbody_component::is_using_gravity,
                  &rigidbody_component::set_is_using_gravity)(
            rttr::metadata("pretty_name", "Use Gravity"),
            rttr::metadata("tooltip", "Simulate gravity for this rigidbody."))
        .property("is_kinematic", &rigidbody_component::is_kinematic, &rigidbody_component::set_is_kinematic)(
            rttr::metadata("pretty_name", "Is Kinematic"),
            rttr::metadata(
                "tooltip",
                "Is the rigidbody kinematic(A rigid body that is not affected by others and can be moved directly.)"))
        .property("mass", &rigidbody_component::get_mass, &rigidbody_component::set_mass)(
            rttr::metadata("min", 0.0f),
            rttr::metadata("pretty_name", "Mass"),
            rttr::metadata("tooltip", "Mass for dynamic rigidbodies."));
}

SAVE(rigidbody_component)
{
    try_save(ar, cereal::make_nvp("is_using_gravity", obj.is_using_gravity()));
    try_save(ar, cereal::make_nvp("is_kinematic", obj.is_kinematic()));
    try_save(ar, cereal::make_nvp("mass", obj.get_mass()));
}
SAVE_INSTANTIATE(rigidbody_component, cereal::oarchive_associative_t);
SAVE_INSTANTIATE(rigidbody_component, cereal::oarchive_binary_t);

LOAD(rigidbody_component)
{
    obj.on_start_load();

    bool is_using_gravity{};
    try_load(ar, cereal::make_nvp("is_using_gravity", is_using_gravity));
    obj.set_is_using_gravity(is_using_gravity);

    bool is_kinematic{};
    try_load(ar, cereal::make_nvp("is_kinematic", is_kinematic));
    obj.set_is_kinematic(is_kinematic);

    float mass{1};
    try_load(ar, cereal::make_nvp("mass", mass));
    obj.set_mass(mass);

    obj.on_end_load();
}

LOAD_INSTANTIATE(rigidbody_component, cereal::iarchive_associative_t);
LOAD_INSTANTIATE(rigidbody_component, cereal::iarchive_binary_t);

} // namespace ace
