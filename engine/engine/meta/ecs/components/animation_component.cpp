#include "animation_component.hpp"
#include <engine/meta/assets/asset_handle.hpp>

#include <serialization/associative_archive.h>
#include <serialization/binary_archive.h>

namespace ace
{
REFLECT(animation_culling_mode)
{
}

REFLECT(animation_component)
{
    rttr::registration::enumeration<animation_component::culling_mode>("animation_component::culling_mode")(
        rttr::value("Always Animate", animation_component::culling_mode::always_animate),
        rttr::value("Renderer Based", animation_component::culling_mode::renderer_based));

    rttr::registration::class_<animation_component>("animation_component")(rttr::metadata("category", "ANIMATION"),
                                                                           rttr::metadata("pretty_name", "Animation"))
        .constructor<>()()
        .property("animation", &animation_component::get_animation, &animation_component::set_animation)(rttr::metadata("pretty_name", "Animation"))
        .property("culling_mode", &animation_component::get_culling_mode, &animation_component::set_culling_mode)(rttr::metadata("pretty_name", "Culling Mode"));
}

SAVE(animation_component)
{
    try_save(ar, ser20::make_nvp("animation", obj.get_animation()));
}
SAVE_INSTANTIATE(animation_component, ser20::oarchive_associative_t);
SAVE_INSTANTIATE(animation_component, ser20::oarchive_binary_t);

LOAD(animation_component)
{
    asset_handle<animation_clip> animation;
    if(try_load(ar, ser20::make_nvp("animation", animation)))
    {
        obj.set_animation(animation);
    }
}
LOAD_INSTANTIATE(animation_component, ser20::iarchive_associative_t);
LOAD_INSTANTIATE(animation_component, ser20::iarchive_binary_t);

} // namespace ace
