#include "id_component.hpp"

#include <serialization/associative_archive.h>
#include <serialization/binary_archive.h>

namespace ace
{
REFLECT(id_component)
{
    rttr::registration::class_<id_component>("id_component")(rttr::metadata("category", "BASIC"),
                                                                           rttr::metadata("pretty_name", "Id"))
        .constructor<>()()
        .property_readonly("id", &id_component::id)(
            rttr::metadata("pretty_name", "Id"),
            rttr::metadata("tooltip",
                           "This is the unique id of the entity."))
        .property("name", &id_component::name)(
            rttr::metadata("pretty_name", "Name"),
            rttr::metadata("tooltip",
                           "This is the name of the entity."))
        ;
}

SAVE(id_component)
{
    try_save(ar, cereal::make_nvp("name", obj.name));
}
SAVE_INSTANTIATE(id_component, cereal::oarchive_associative_t);
SAVE_INSTANTIATE(id_component, cereal::oarchive_binary_t);

LOAD(id_component)
{
    try_load(ar, cereal::make_nvp("name", obj.name));
}
LOAD_INSTANTIATE(id_component, cereal::iarchive_associative_t);
LOAD_INSTANTIATE(id_component, cereal::iarchive_binary_t);
} // namespace ace
