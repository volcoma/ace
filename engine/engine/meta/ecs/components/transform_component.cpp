#include "transform_component.hpp"

#include "../../core/math/transform.hpp"
#include "../../core/math/vector.hpp"
#include "../entity.hpp"

#include <serialization/associative_archive.h>
#include <serialization/binary_archive.h>
#include <serialization/types/vector.hpp>

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
                           "Affected by parent transformation."));
}

SAVE(transform_component)
{
    bool is_root = obj.get_owner().all_of<root_component>();

    try_save(ar, ser20::make_nvp("local_transform",  obj.get_transform_local()));
    try_save(ar, ser20::make_nvp("parent", is_root ? entt::handle{} : obj.get_parent()));
    try_save(ar, ser20::make_nvp("children", obj.get_children()));
}
SAVE_INSTANTIATE(transform_component, ser20::oarchive_associative_t);
SAVE_INSTANTIATE(transform_component, ser20::oarchive_binary_t);

LOAD(transform_component)
{
    math::transform local_transform;
    try_load(ar, ser20::make_nvp("local_transform", local_transform));

    obj.set_transform_local(local_transform);

    entt::handle parent;
    try_load(ar, ser20::make_nvp("parent", parent));

    set_parent_params params;
    params.local_transform_stays = true;
    params.global_transform_stays = false;
    obj.set_parent(parent, params);
}
LOAD_INSTANTIATE(transform_component, ser20::iarchive_associative_t);
LOAD_INSTANTIATE(transform_component, ser20::iarchive_binary_t);

} // namespace ace
