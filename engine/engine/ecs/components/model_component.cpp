#include "model_component.h"
#include "transform_component.h"

namespace ace
{
void model_component::set_casts_shadow(bool cast_shadow)
{
    if(casts_shadow_ == cast_shadow)
    {
        return;
    }

    touch();

    casts_shadow_ = cast_shadow;
}

void model_component::set_static(bool is_static)
{
    if(static_ == is_static)
    {
        return;
    }

    touch();

    static_ = is_static;
}

void model_component::set_casts_reflection(bool casts_reflection)
{
    if(casts_reflection_ == casts_reflection)
    {
        return;
    }

    touch();

    casts_reflection_ = casts_reflection;
}

auto model_component::casts_shadow() const -> bool
{
    return casts_shadow_;
}

auto model_component::is_static() const -> bool
{
    return static_;
}

auto model_component::get_model() const -> const model&
{
    return model_;
}

void model_component::set_model(const model& model)
{
    model_ = model;

    touch();
}

void model_component::set_bone_transforms(const std::vector<math::transform>& bone_transforms)
{
    bone_transforms_ = bone_transforms;

    touch();
}

auto model_component::get_bone_transforms() const -> const std::vector<math::transform>&
{
    return bone_transforms_;
}

void model_component::set_bone_entities(const std::vector<entt::handle>& bone_entities)
{
    bone_entities_ = bone_entities;

    touch();
}

auto model_component::get_bone_entities() const -> const std::vector<entt::handle>&
{
    return bone_entities_;
}

auto model_component::casts_reflection() const -> bool
{
    return casts_reflection_;
}

} // namespace ace
