#include "id_component.hpp"
#include "hpp/uuid.hpp"

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
        ;
}

REFLECT(tag_component)
{
    rttr::registration::class_<tag_component>("tag_component")(rttr::metadata("category", "BASIC"),
                                                                           rttr::metadata("pretty_name", "Tag"))
        .constructor<>()()
        .property("tag", &tag_component::tag)(
            rttr::metadata("pretty_name", "Tag"),
            rttr::metadata("tooltip",
                           "This is the name of the entity."))
        ;
}

SAVE(id_component)
{

    try_save(ar, cereal::make_nvp("id", hpp::to_string(obj.id)));
}
SAVE_INSTANTIATE(id_component, cereal::oarchive_associative_t);
SAVE_INSTANTIATE(id_component, cereal::oarchive_binary_t);

SAVE(tag_component)
{
    try_save(ar, cereal::make_nvp("tag", obj.tag));
}
SAVE_INSTANTIATE(tag_component, cereal::oarchive_associative_t);
SAVE_INSTANTIATE(tag_component, cereal::oarchive_binary_t);

LOAD(id_component)
{
    std::string suuid;

    try_load(ar, cereal::make_nvp("id", suuid));

    auto id = hpp::uuid::from_string(suuid);
    obj.id = id.value_or(hpp::uuid{});
}

LOAD_INSTANTIATE(id_component, cereal::iarchive_associative_t);
LOAD_INSTANTIATE(id_component, cereal::iarchive_binary_t);

LOAD(tag_component)
{
    try_load(ar, cereal::make_nvp("tag", obj.tag));
}

LOAD_INSTANTIATE(tag_component, cereal::iarchive_associative_t);
LOAD_INSTANTIATE(tag_component, cereal::iarchive_binary_t);
} // namespace ace
