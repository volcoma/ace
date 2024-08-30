#include "model_component.hpp"

#include <engine/meta/ecs/entity.hpp>
#include <engine/meta/rendering/material.hpp>
#include <engine/meta/rendering/mesh.hpp>
#include <engine/meta/rendering/model.hpp>

#include <serialization/associative_archive.h>
#include <serialization/binary_archive.h>
#include <serialization/types/vector.hpp>

namespace ace
{

REFLECT(model_component)
{
    rttr::registration::class_<model_component>("model_component")(rttr::metadata("category", "RENDERING"),
                                                                   rttr::metadata("pretty_name", "Model"))
        .constructor<>()
        .property("static", &model_component::is_static, &model_component::set_static)(
            rttr::metadata("pretty_name", "Static"))
        .property("casts_shadow", &model_component::casts_shadow, &model_component::set_casts_shadow)(
            rttr::metadata("pretty_name", "Casts Shadow"))
        .property("casts_reflection", &model_component::casts_reflection, &model_component::set_casts_reflection)(
            rttr::metadata("pretty_name", "Casts Reflection"))
        .property("model", &model_component::get_model, &model_component::set_model)(
            rttr::metadata("pretty_name", "Model"));
}

SAVE(model_component)
{
    try_save(ar, cereal::make_nvp("static", obj.is_static()));
    try_save(ar, cereal::make_nvp("casts_shadow", obj.casts_shadow()));
    try_save(ar, cereal::make_nvp("casts_reflection", obj.casts_reflection()));
    try_save(ar, cereal::make_nvp("model", obj.get_model()));
    try_save(ar, cereal::make_nvp("bone_entities", obj.get_bone_entities()));
    try_save(ar, cereal::make_nvp("submesh_entities", obj.get_submesh_entities()));

}
SAVE_INSTANTIATE(model_component, cereal::oarchive_associative_t);
SAVE_INSTANTIATE(model_component, cereal::oarchive_binary_t);

LOAD(model_component)
{
    bool is_static{};
    try_load(ar, cereal::make_nvp("static", is_static));
    obj.set_static(is_static);

    bool casts_shadow{};
    try_load(ar, cereal::make_nvp("casts_shadow", casts_shadow));
    obj.set_casts_shadow(casts_shadow);

    bool casts_reflection{};
    try_load(ar, cereal::make_nvp("casts_reflection", casts_reflection));
    obj.set_casts_reflection(casts_reflection);

    model mod;
    try_load(ar, cereal::make_nvp("model", mod));
    obj.set_model(mod);

    std::vector<entt::handle> bone_entities;
    try_load(ar, cereal::make_nvp("bone_entities", bone_entities));
    obj.set_bone_entities(bone_entities);


    std::vector<entt::handle> submesh_entities;
    try_load(ar, cereal::make_nvp("submesh_entities", submesh_entities));
    obj.set_submesh_entities(submesh_entities);
}
LOAD_INSTANTIATE(model_component, cereal::iarchive_associative_t);
LOAD_INSTANTIATE(model_component, cereal::iarchive_binary_t);


REFLECT(bone_component)
{
    rttr::registration::class_<bone_component>("bone_component")(rttr::metadata("category", "RENDERING"),
                                                                 rttr::metadata("pretty_name", "Bone"))
        .constructor<>();
}

SAVE(bone_component)
{

}
SAVE_INSTANTIATE(bone_component, cereal::oarchive_associative_t);
SAVE_INSTANTIATE(bone_component, cereal::oarchive_binary_t);

LOAD(bone_component)
{

}
LOAD_INSTANTIATE(bone_component, cereal::iarchive_associative_t);
LOAD_INSTANTIATE(bone_component, cereal::iarchive_binary_t);

REFLECT(subset_component)
{
    rttr::registration::class_<subset_component>("subset_component")(rttr::metadata("category", "RENDERING"),
                                                                    rttr::metadata("pretty_name", "Subset"))
        .constructor<>()
        .property_readonly("subsets", &subset_component::subsets)(
            rttr::metadata("pretty_name", "Subsets"),
            rttr::metadata("tooltip", "Subsets affected by this node."))
        ;
}

SAVE(subset_component)
{
    try_save(ar, cereal::make_nvp("subsets", obj.subsets));

}
SAVE_INSTANTIATE(subset_component, cereal::oarchive_associative_t);
SAVE_INSTANTIATE(subset_component, cereal::oarchive_binary_t);

LOAD(subset_component)
{
    try_load(ar, cereal::make_nvp("subsets", obj.subsets));
}
LOAD_INSTANTIATE(subset_component, cereal::iarchive_associative_t);
LOAD_INSTANTIATE(subset_component, cereal::iarchive_binary_t);


} // namespace ace
