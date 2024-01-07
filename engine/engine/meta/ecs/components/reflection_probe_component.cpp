#include "reflection_probe_component.hpp"
#include <engine/meta/core/math/vector.hpp>
#include <engine/meta/rendering/reflection_probe.hpp>

#include <serialization/associative_archive.h>
#include <serialization/binary_archive.h>

namespace ace
{
REFLECT(reflection_probe_component)
{
    rttr::registration::class_<reflection_probe_component>("reflection_probe_component")(
        rttr::metadata("category", "LIGHTING"),
        rttr::metadata("pretty_name", "Reflection Probe"))
        .constructor<>()
        .property("probe", &reflection_probe_component::get_probe, &reflection_probe_component::set_probe)(
            rttr::metadata("pretty_name", "Probe"));
    ;
}

SAVE(reflection_probe_component)
{
    try_save(ar, cereal::make_nvp("probe", obj.get_probe()));
}
SAVE_INSTANTIATE(reflection_probe_component, cereal::oarchive_associative_t);
SAVE_INSTANTIATE(reflection_probe_component, cereal::oarchive_binary_t);

LOAD(reflection_probe_component)
{
    reflection_probe probe;
    try_load(ar, cereal::make_nvp("probe", probe));
    obj.set_probe(probe);
}
LOAD_INSTANTIATE(reflection_probe_component, cereal::iarchive_associative_t);
LOAD_INSTANTIATE(reflection_probe_component, cereal::iarchive_binary_t);
} // namespace ace
