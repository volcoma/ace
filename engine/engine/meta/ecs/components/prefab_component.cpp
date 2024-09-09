#include "prefab_component.hpp"
#include <engine/meta/assets/asset_handle.hpp>

#include <serialization/associative_archive.h>
#include <serialization/binary_archive.h>

namespace ace
{

REFLECT(prefab_component)
{
    rttr::registration::class_<prefab_component>("prefab_component")(rttr::metadata("category", "BASIC"),
                                                                     rttr::metadata("pretty_name", "Prefab"))
        .constructor<>()()
        .property("source", &prefab_component::source)(rttr::metadata("pretty_name", "Source"));
}

SAVE(prefab_component)
{
    try_save(ar, ser20::make_nvp("source", obj.source));
}
SAVE_INSTANTIATE(prefab_component, ser20::oarchive_associative_t);
SAVE_INSTANTIATE(prefab_component, ser20::oarchive_binary_t);

LOAD(prefab_component)
{
    try_load(ar, ser20::make_nvp("source", obj.source));
}
LOAD_INSTANTIATE(prefab_component, ser20::iarchive_associative_t);
LOAD_INSTANTIATE(prefab_component, ser20::iarchive_binary_t);
} // namespace ace
