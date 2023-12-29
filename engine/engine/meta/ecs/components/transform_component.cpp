#include "transform_component.hpp"

#include "../../core/math/transform.hpp"
#include "../../core/math/vector.hpp"
#include "../entity.hpp"

#include <serialization/types/vector.hpp>
#include <serialization/associative_archive.h>
#include <serialization/binary_archive.h>

namespace ace
{
REFLECT(transform_component)
{
    rttr::registration::class_<transform_component>("transform_component")(rttr::metadata("category", "RENDERING"),
                                                                           rttr::metadata("pretty_name", "Transform"))
        .constructor<>()()
        .property("local", &transform_component::get_transform_local, &transform_component::set_transform_local)(
            rttr::metadata("pretty_name", "Local"),
            rttr::metadata("tooltip",
                           "This is the local transformation.\n"
                           "It is relative to the parent."))
        .property("world", &transform_component::get_transform_global, &transform_component::set_transform_global)(
            rttr::metadata("pretty_name", "World"),
            rttr::metadata("tooltip",
                           "This is the world transformation.\n"
                           "Affected by parent transformation."))
            ;
}

SAVE(transform_component)
{
    try_save(ar, cereal::make_nvp("local_transform", obj.get_transform_local()));
    try_save(ar, cereal::make_nvp("parent", obj.get_parent()));
    try_save(ar, cereal::make_nvp("children", obj.get_children()));
}
SAVE_INSTANTIATE(transform_component, cereal::oarchive_associative_t);
SAVE_INSTANTIATE(transform_component, cereal::oarchive_binary_t);

LOAD(transform_component)
{
    math::transform local_transform;
    try_load(ar, cereal::make_nvp("local_transform", local_transform));

    obj.set_transform_local(local_transform);


    auto& rel = obj.get_owner().get_or_emplace<relationship_component>();
    try_load(ar, cereal::make_nvp("parent", rel.parent));

    std::vector<entt::handle> children;
    try_load(ar, cereal::make_nvp("children", rel.children));

}
LOAD_INSTANTIATE(transform_component, cereal::iarchive_associative_t);
LOAD_INSTANTIATE(transform_component, cereal::iarchive_binary_t);

} // namespace ace
