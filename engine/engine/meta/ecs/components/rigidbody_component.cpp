#include "rigidbody_component.hpp"

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
                  &rigidbody_component::get_is_using_gravity,
                  &rigidbody_component::set_is_using_gravity)(
            rttr::metadata("pretty_name", "Using Gravity"),
            rttr::metadata("tooltip", "Simulate gravity for this rigidbody."));
}

SAVE(rigidbody_component)
{
    try_save(ar, cereal::make_nvp("is_using_gravity", obj.get_is_using_gravity()));
}
SAVE_INSTANTIATE(rigidbody_component, cereal::oarchive_associative_t);
SAVE_INSTANTIATE(rigidbody_component, cereal::oarchive_binary_t);

LOAD(rigidbody_component)
{
    bool is_using_gravity{};
    try_load(ar, cereal::make_nvp("is_using_gravity", is_using_gravity));
    obj.set_is_using_gravity(is_using_gravity);
}

LOAD_INSTANTIATE(rigidbody_component, cereal::iarchive_associative_t);
LOAD_INSTANTIATE(rigidbody_component, cereal::iarchive_binary_t);

} // namespace ace
