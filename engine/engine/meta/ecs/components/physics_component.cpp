#include "physics_component.hpp"
#include <engine/ecs/components/physics_component.h>
#include <engine/meta/core/math/vector.hpp>

#include <cereal/types/variant.hpp>
#include <serialization/associative_archive.h>
#include <serialization/binary_archive.h>

namespace ace
{

REFLECT(physics_box_shape)
{
    rttr::registration::class_<physics_box_shape>("physics_box_shape")(rttr::metadata("category", "PHYSICS"),
                                                                       rttr::metadata("pretty_name", "Box"))
        .constructor<>()()
        .property("extends",
                  &physics_box_shape::extends)(rttr::metadata("pretty_name", "Extends"),
                                               rttr::metadata("tooltip", "The extends of the box collider."));
}

SAVE(physics_box_shape)
{
    try_save(ar, cereal::make_nvp("extends", obj.extends));
}
SAVE_INSTANTIATE(physics_box_shape, cereal::oarchive_associative_t);
SAVE_INSTANTIATE(physics_box_shape, cereal::oarchive_binary_t);

LOAD(physics_box_shape)
{
    try_load(ar, cereal::make_nvp("extends", obj.extends));
}

LOAD_INSTANTIATE(physics_box_shape, cereal::iarchive_associative_t);
LOAD_INSTANTIATE(physics_box_shape, cereal::iarchive_binary_t);


REFLECT(physics_compound_shape)
{
    rttr::registration::class_<physics_compound_shape>("physics_compound_shape")(rttr::metadata("category", "PHYSICS"),
                                                                                 rttr::metadata("pretty_name", "Shape"))
        .constructor<>()();
}

SAVE(physics_compound_shape)
{
    try_save(ar, cereal::make_nvp("shape", obj.shape));
}
SAVE_INSTANTIATE(physics_compound_shape, cereal::oarchive_associative_t);
SAVE_INSTANTIATE(physics_compound_shape, cereal::oarchive_binary_t);

LOAD(physics_compound_shape)
{
    try_load(ar, cereal::make_nvp("shape", obj.shape));
}

LOAD_INSTANTIATE(physics_compound_shape, cereal::iarchive_associative_t);
LOAD_INSTANTIATE(physics_compound_shape, cereal::iarchive_binary_t);

REFLECT(phyisics_component)
{
    rttr::registration::class_<phyisics_component>("physics_component")(rttr::metadata("category", "PHYSICS"),
                                                                           rttr::metadata("pretty_name", "Physics"))
        .constructor<>()()
        .property("is_using_gravity",
                  &phyisics_component::is_using_gravity,
                  &phyisics_component::set_is_using_gravity)(
            rttr::metadata("pretty_name", "Use Gravity"),
            rttr::metadata("tooltip", "Simulate gravity for this rigidbody."))
        .property("is_kinematic", &phyisics_component::is_kinematic, &phyisics_component::set_is_kinematic)(
            rttr::metadata("pretty_name", "Is Kinematic"),
            rttr::metadata(
                "tooltip",
                "Is the rigidbody kinematic(A rigid body that is not affected by others and can be moved directly.)"))
        .property("mass", &phyisics_component::get_mass, &phyisics_component::set_mass)(
            rttr::metadata("min", 0.0f),
            rttr::metadata("pretty_name", "Mass"),
            rttr::metadata("tooltip", "Mass for dynamic rigidbodies."))
        .property("shape", &phyisics_component::get_shape, &phyisics_component::set_shape)(
            rttr::metadata("pretty_name", "Shape"),
            rttr::metadata("tooltip", "Shape."));
}

SAVE(phyisics_component)
{
    try_save(ar, cereal::make_nvp("is_using_gravity", obj.is_using_gravity()));
    try_save(ar, cereal::make_nvp("is_kinematic", obj.is_kinematic()));
    try_save(ar, cereal::make_nvp("mass", obj.get_mass()));
    try_save(ar, cereal::make_nvp("shapes", obj.get_shape()));
}
SAVE_INSTANTIATE(physics_component, cereal::oarchive_associative_t);
SAVE_INSTANTIATE(physics_component, cereal::oarchive_binary_t);

LOAD(phyisics_component)
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

    std::vector<physics_compound_shape> shapes;
    try_load(ar, cereal::make_nvp("shapes", shapes));
    obj.set_shape(shapes);

    obj.on_end_load();
}

LOAD_INSTANTIATE(physics_component, cereal::iarchive_associative_t);
LOAD_INSTANTIATE(physics_component, cereal::iarchive_binary_t);

} // namespace ace
