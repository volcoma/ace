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
    rttr::registration::enumeration<skylight_component::sky_mode>("sky_mode")(
        rttr::value("Standard", skylight_component::sky_mode::standard),
        rttr::value("Perez", skylight_component::sky_mode::perez));

    rttr::registration::class_<skylight_component>("skylight_component")(rttr::metadata("category", "LIGHTING"),
                                                                         rttr::metadata("pretty_name", "Skylight"))
        .constructor<>()
        .property("mode", &skylight_component::get_mode, &skylight_component::set_mode)(
            rttr::metadata("pretty_name", "Mode"))
        .property("turbidity", &skylight_component::get_turbidity, &skylight_component::set_turbidity)(
            rttr::metadata("pretty_name", "Turbidity"),
            rttr::metadata("min", 1.9f),
            rttr::metadata("max", 10.0f),
            rttr::metadata(
                "tooltip",
                "Adjusts the clarity of the atmosphere. Lower values (1.9) result in a clear, blue sky, while higher "
                "values (up to 10) create a hazy, diffused appearance with more scattering of light.."));
}

SAVE(skylight_component)
{
    try_save(ar, cereal::make_nvp("mode", obj.get_mode()));
    try_save(ar, cereal::make_nvp("turbidity", obj.get_turbidity()));

}
SAVE_INSTANTIATE(skylight_component, cereal::oarchive_associative_t);
SAVE_INSTANTIATE(skylight_component, cereal::oarchive_binary_t);

LOAD(skylight_component)
{
    skylight_component::sky_mode mode;
    if(try_load(ar, cereal::make_nvp("mode", mode)))
    {
        obj.set_mode(mode);
    }

    float turbidity{};
    if(try_load(ar, cereal::make_nvp("turbidity", turbidity)))
    {
        obj.set_turbidity(turbidity);
    }
}
LOAD_INSTANTIATE(skylight_component, cereal::iarchive_associative_t);
LOAD_INSTANTIATE(skylight_component, cereal::iarchive_binary_t);
} // namespace ace
