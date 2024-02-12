#include "box_collider_component.hpp"

#include <engine/meta/core/math/vector.hpp>
#include <serialization/associative_archive.h>
#include <serialization/binary_archive.h>

namespace ace
{
REFLECT(box_collider_component)
{
    rttr::registration::class_<box_collider_component>("box_collider_component")(rttr::metadata("category", "PHYSICS"),
                                                                           rttr::metadata("pretty_name", "Box Collider"))
        .constructor<>()()
        .property("extends",
                  &box_collider_component::get_extends,
                  &box_collider_component::set_extends)(
            rttr::metadata("pretty_name", "Extends"),
            rttr::metadata("tooltip", "The extends of the box collider."));
}

SAVE(box_collider_component)
{
    try_save(ar, cereal::make_nvp("extends", obj.get_extends()));
}
SAVE_INSTANTIATE(box_collider_component, cereal::oarchive_associative_t);
SAVE_INSTANTIATE(box_collider_component, cereal::oarchive_binary_t);

LOAD(box_collider_component)
{
    math::vec3 extends{};
    try_load(ar, cereal::make_nvp("extends", extends));
    obj.set_extends(extends);
}

LOAD_INSTANTIATE(box_collider_component, cereal::iarchive_associative_t);
LOAD_INSTANTIATE(box_collider_component, cereal::iarchive_binary_t);

} // namespace ace
