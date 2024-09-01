#include "animation_component.hpp"
#include <engine/meta/assets/asset_handle.hpp>

#include <serialization/associative_archive.h>
#include <serialization/binary_archive.h>

namespace ace
{
REFLECT(animation_component)
{
    rttr::registration::class_<animation_component>("animation_component")(rttr::metadata("category", "ANIMATION"),
                                                                 rttr::metadata("pretty_name", "Animation"))
        .constructor<>()()
        .property("animation", &animation_component::animation);
}

SAVE(animation_component)
{
    try_save(ar, cereal::make_nvp("animation", obj.animation));

}
SAVE_INSTANTIATE(animation_component, cereal::oarchive_associative_t);
SAVE_INSTANTIATE(animation_component, cereal::oarchive_binary_t);

LOAD(animation_component)
{
    try_load(ar, cereal::make_nvp("animation", obj.animation));
}
LOAD_INSTANTIATE(animation_component, cereal::iarchive_associative_t);
LOAD_INSTANTIATE(animation_component, cereal::iarchive_binary_t);

} // namespace ace
