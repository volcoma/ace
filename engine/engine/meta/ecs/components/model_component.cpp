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
    try_save(ar, ser20::make_nvp("static", obj.is_static()));
    try_save(ar, ser20::make_nvp("casts_shadow", obj.casts_shadow()));
    try_save(ar, ser20::make_nvp("casts_reflection", obj.casts_reflection()));
    try_save(ar, ser20::make_nvp("model", obj.get_model()));

}
SAVE_INSTANTIATE(model_component, ser20::oarchive_associative_t);
SAVE_INSTANTIATE(model_component, ser20::oarchive_binary_t);

LOAD(model_component)
{
    bool is_static{};
    try_load(ar, ser20::make_nvp("static", is_static));
    obj.set_static(is_static);

    bool casts_shadow{};
    try_load(ar, ser20::make_nvp("casts_shadow", casts_shadow));
    obj.set_casts_shadow(casts_shadow);

    bool casts_reflection{};
    try_load(ar, ser20::make_nvp("casts_reflection", casts_reflection));
    obj.set_casts_reflection(casts_reflection);

    model mod;
    try_load(ar, ser20::make_nvp("model", mod));
    obj.set_model(mod);
}
LOAD_INSTANTIATE(model_component, ser20::iarchive_associative_t);
LOAD_INSTANTIATE(model_component, ser20::iarchive_binary_t);


REFLECT(bone_component)
{
    rttr::registration::class_<bone_component>("bone_component")(rttr::metadata("category", "RENDERING"),
                                                                 rttr::metadata("pretty_name", "Bone"))
        .constructor<>()
        .property_readonly("bone_index", &bone_component::bone_index)(
            rttr::metadata("pretty_name", "Bone Index"),
            rttr::metadata("tooltip", "The bone index this object represents."));
}

SAVE(bone_component)
{
    try_save(ar, ser20::make_nvp("bone_index", obj.bone_index));
}
SAVE_INSTANTIATE(bone_component, ser20::oarchive_associative_t);
SAVE_INSTANTIATE(bone_component, ser20::oarchive_binary_t);

LOAD(bone_component)
{
    try_load(ar, ser20::make_nvp("bone_index", obj.bone_index));
}
LOAD_INSTANTIATE(bone_component, ser20::iarchive_associative_t);
LOAD_INSTANTIATE(bone_component, ser20::iarchive_binary_t);

REFLECT(submesh_component)
{
    rttr::registration::class_<submesh_component>("submesh_component")(rttr::metadata("category", "RENDERING"),
                                                                    rttr::metadata("pretty_name", "Submesh"))
        .constructor<>()
        .property_readonly("submeshes", &submesh_component::submeshes)(
            rttr::metadata("pretty_name", "Submeshes"),
            rttr::metadata("tooltip", "Submeshes affected by this node."))
        ;
}

SAVE(submesh_component)
{
    try_save(ar, ser20::make_nvp("submeshes", obj.submeshes));

}
SAVE_INSTANTIATE(submesh_component, ser20::oarchive_associative_t);
SAVE_INSTANTIATE(submesh_component, ser20::oarchive_binary_t);

LOAD(submesh_component)
{
    try_load(ar, ser20::make_nvp("submeshes", obj.submeshes));
}
LOAD_INSTANTIATE(submesh_component, ser20::iarchive_associative_t);
LOAD_INSTANTIATE(submesh_component, ser20::iarchive_binary_t);


} // namespace ace
