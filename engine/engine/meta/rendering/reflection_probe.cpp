#include "reflection_probe.hpp"
#include <engine/meta/core/math/vector.hpp>

#include <serialization/associative_archive.h>
#include <serialization/binary_archive.h>

namespace ace
{
REFLECT(reflection_probe)
{
    rttr::registration::enumeration<probe_type>("probe_type")(rttr::value("Box", probe_type::box),
                                                              rttr::value("Sphere", probe_type::sphere));
    rttr::registration::enumeration<reflect_method>("reflect_method")(
        rttr::value("Environment", reflect_method::environment),
        rttr::value("Static Only", reflect_method::static_only));
    rttr::registration::class_<reflection_probe::box>("box")
        .property("extents", &reflection_probe::box::extents)(rttr::metadata("pretty_name", "Extents"))
        .property("transition_distance",
                  &reflection_probe::box::transition_distance)(rttr::metadata("pretty_name", "Transition Distance"));
    rttr::registration::class_<reflection_probe::sphere>("sphere").property("range", &reflection_probe::sphere::range)(
        rttr::metadata("pretty_name", "Range"));
    rttr::registration::class_<reflection_probe>("reflection_probe")
        .property("type", &reflection_probe::type)(rttr::metadata("pretty_name", "Type"))
        .property("method", &reflection_probe::method)(rttr::metadata("pretty_name", "Method"));
}

SAVE(reflection_probe)
{
    try_save(ar, cereal::make_nvp("type", obj.type));
    try_save(ar, cereal::make_nvp("method", obj.method));
    try_save(ar, cereal::make_nvp("extents", obj.box_data.extents));
    try_save(ar, cereal::make_nvp("transition_distance", obj.box_data.transition_distance));
    try_save(ar, cereal::make_nvp("range", obj.sphere_data.range));
}
SAVE_INSTANTIATE(reflection_probe, cereal::oarchive_associative_t);
SAVE_INSTANTIATE(reflection_probe, cereal::oarchive_binary_t);

LOAD(reflection_probe)
{
    try_load(ar, cereal::make_nvp("type", obj.type));
    try_load(ar, cereal::make_nvp("method", obj.method));
    try_load(ar, cereal::make_nvp("extents", obj.box_data.extents));
    try_load(ar, cereal::make_nvp("transition_distance", obj.box_data.transition_distance));
    try_load(ar, cereal::make_nvp("range", obj.sphere_data.range));
}
LOAD_INSTANTIATE(reflection_probe, cereal::iarchive_associative_t);
LOAD_INSTANTIATE(reflection_probe, cereal::iarchive_binary_t);
} // namespace ace
