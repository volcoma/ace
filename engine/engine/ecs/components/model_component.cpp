#include "model_component.h"
#include "id_component.h"
#include "transform_component.h"

namespace ace
{
namespace
{

auto process_node_impl(const std::unique_ptr<mesh::armature_node>& node,
                       const skin_bind_data& bind_data,
                       entt::handle& parent,
                       std::vector<entt::handle>& entity_nodes) -> entt::handle
{
    auto entity_node = parent;

    auto& trans_comp = parent.get<transform_component>();
    const auto& children = trans_comp.get_children();
    for(const auto& child : children)
    {
        if(child.get<tag_component>().tag == node->name)
        {
            entity_node = child;
            break;
        }
    }


    if(entity_node == parent)
    {
        auto& reg = *entity_node.registry();
        entity_node = scene::create_entity(reg, node->name, parent);
        auto& trans_comp = entity_node.get<transform_component>();
        trans_comp.set_transform_local(node->local_transform);
    }

    return entity_node;
}

void process_node(const std::unique_ptr<mesh::armature_node>& node,
                  const skin_bind_data& bind_data,
                  entt::handle parent,
                  std::vector<entt::handle>& entity_nodes)
{
    if(!parent)
        return;

    auto entity_node = parent;
    entity_node = process_node_impl(node, bind_data, entity_node, entity_nodes);
    auto bone = bind_data.find_bone_by_id(node->name);

    if(!node->subsets.empty())
    {
        auto& comp = entity_node.emplace<subset_component>();
        comp.subsets = node->subsets;
    }

    if(bone)
    {
        entity_node.emplace<bone_component>();
        entity_nodes.push_back(entity_node);
    }

    for(auto& child : node->children)
    {
        process_node(child, bind_data, entity_node, entity_nodes);
    }

}

void process_armature(const std::unique_ptr<mesh::armature_node>& root,
                      const skin_bind_data& bind_data,
                      entt::handle parent,
                      std::vector<entt::handle>& entity_nodes)
{

    process_node(root, bind_data, parent, entity_nodes);
}
} // namespace

void model_component::update_armature()
{
    if(!get_bone_entities().empty())
    {
        return;
    }

    auto lod = model_.get_lod(0);
    if(!lod)
    {
        return;
    }
    
    const auto& mesh = lod.get();

    const auto& skin_data = mesh->get_skin_bind_data();

    // Has skinning data?
    if(skin_data.has_bones())
    {
        const auto& node = mesh->get_armature();
        auto owner = get_owner();

        std::vector<entt::handle> bone_entities;
        process_armature(node, skin_data, owner, bone_entities);
        set_bone_entities(bone_entities);
        set_static(false);
    }
}

void model_component::on_create_component(entt::registry& r, const entt::entity e)
{
    entt::handle entity(r, e);

    auto& component = entity.get<model_component>();
    component.set_owner(entity);

    component.set_bone_transforms({});
    component.set_bone_entities({});
}

void model_component::on_destroy_component(entt::registry& r, const entt::entity e)
{
    entt::handle entity(r, e);

    auto& component = entity.get<model_component>();
}

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
