#include "light_component.hpp"

#include <engine/meta/core/math/vector.hpp>
#include <engine/meta/rendering/light.hpp>

#include <serialization/associative_archive.h>
#include <serialization/binary_archive.h>

namespace ace
{
REFLECT(light_component)
{
    rttr::registration::class_<light_component>("light_component")(rttr::metadata("category", "LIGHTING"),
                                                                   rttr::metadata("pretty_name", "Light"))
        .constructor<>()
        .property("light", &light_component::get_light, &light_component::set_light)(
            rttr::metadata("pretty_name", "Light"));
}

SAVE(light_component)
{
    try_save(ar, cereal::make_nvp("light", obj.get_light()));
}
SAVE_INSTANTIATE(light_component, cereal::oarchive_associative_t);
SAVE_INSTANTIATE(light_component, cereal::oarchive_binary_t);

LOAD(light_component)
{
    light l;
    try_load(ar, cereal::make_nvp("light", l));
    obj.set_light(l);
}
LOAD_INSTANTIATE(light_component, cereal::iarchive_associative_t);
LOAD_INSTANTIATE(light_component, cereal::iarchive_binary_t);

REFLECT(skylight_component)
{
    rttr::registration::class_<skylight_component>("skylight_component")(rttr::metadata("category", "LIGHTING"),
                                                                         rttr::metadata("pretty_name", "Skylight"))
        .constructor<>();
}

SAVE(skylight_component)
{
}
SAVE_INSTANTIATE(skylight_component, cereal::oarchive_associative_t);
SAVE_INSTANTIATE(skylight_component, cereal::oarchive_binary_t);

LOAD(skylight_component)
{
}
LOAD_INSTANTIATE(skylight_component, cereal::iarchive_associative_t);
LOAD_INSTANTIATE(skylight_component, cereal::iarchive_binary_t);
} // namespace ace
