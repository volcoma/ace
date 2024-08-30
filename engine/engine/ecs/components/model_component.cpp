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
                       std::vector<entt::handle>& bone_nodes,
                       std::vector<entt::handle>& submesh_nodes) -> entt::handle
{
    auto entity_node = parent;

    if(entity_node == parent)
    {
        auto& reg = *entity_node.registry();
        entity_node = scene::create_entity(reg, node->name, parent);
        auto& trans_comp = entity_node.get<transform_component>();
        trans_comp.set_transform_local(node->local_transform);

        if(!node->subsets.empty())
        {
            auto& comp = entity_node.emplace<subset_component>();
            comp.subsets = node->subsets;
            submesh_nodes.push_back(entity_node);
        }

        auto bone = bind_data.find_bone_by_id(node->name);
        if(bone)
        {
            entity_node.emplace<bone_component>();
            bone_nodes.push_back(entity_node);
        }
    }

    return entity_node;
}

void process_node(const std::unique_ptr<mesh::armature_node>& node,
                  const skin_bind_data& bind_data,
                  entt::handle parent,
                  std::vector<entt::handle>& bone_nodes,
                  std::vector<entt::handle>& submesh_nodes)
{
    if(!parent)
        return;

    auto entity_node = process_node_impl(node, bind_data, parent, bone_nodes, submesh_nodes);

    for(auto& child : node->children)
    {
        process_node(child, bind_data, entity_node, bone_nodes, submesh_nodes);
    }
}

auto process_armature(const mesh& render_mesh,
                      entt::handle parent,
                      std::vector<entt::handle>& bone_nodes,
                      std::vector<entt::handle>& submesh_nodes) -> bool
{
    const auto& root = render_mesh.get_armature();
    if(!root)
    {
        return false;
    }

    const auto& skin_data = render_mesh.get_skin_bind_data();
    process_node(root, skin_data, parent, bone_nodes, submesh_nodes);

    return true;
}

auto get_transforms_for_entities(const std::vector<entt::handle>& entities) -> std::vector<math::transform>
{
    std::vector<math::transform> result;
    if(!entities.empty())
    {
        result.reserve(entities.size());
        for(const auto& e : entities)
        {
            if(e)
            {
                const auto transform_comp = e.try_get<transform_component>();
                if(transform_comp)
                {
                    result.emplace_back(transform_comp->get_transform_global());
                    continue;
                }
            }

            result.emplace_back();
        }
    }

    return result;
}

auto get_transforms_for_submeshes(const std::vector<entt::handle>& entities, uint32_t subsets_count)
    -> std::vector<math::transform>
{
    std::vector<math::transform> result;
    if(!entities.empty())
    {
        result.resize(subsets_count, math::transform::identity());

        for(size_t i = 0; i < entities.size(); ++i)
        {
            const auto& e = entities[i];
            if(e)
            {
                const auto submesh_comp = e.try_get<subset_component>();

                const auto transform_comp = e.try_get<transform_component>();
                if(submesh_comp && transform_comp)
                {
                    for(const auto& submesh_id : submesh_comp->subsets)
                    {
                        result[submesh_id] = transform_comp->get_transform_global();
                    }

                    continue;
                }
            }
        }
    }

    return result;
}
} // namespace

void model_component::create_armature()
{
    bool has_processed_bones = !get_bone_entities().empty();
    bool has_processed_submeshes = !get_submesh_entities().empty();

    auto lod = model_.get_lod(0);
    if(!lod)
    {
        return;
    }

    if(!has_processed_submeshes)
    {
        const auto& mesh = lod.get();

        auto owner = get_owner();

        std::vector<entt::handle> bone_entities;
        std::vector<entt::handle> submesh_entities;
        if(process_armature(*mesh, owner, bone_entities, submesh_entities))
        {
            set_submesh_entities(submesh_entities);

            if(!has_processed_bones)
            {
                const auto& skin_data = mesh->get_skin_bind_data();
                // Has skinning data?
                if(skin_data.has_bones())
                {
                    set_bone_entities(bone_entities);
                    set_static(false);
                }
            }
        }

    }
}

void model_component::update_armature()
{
    create_armature();

    auto lod = model_.get_lod(0);
    if(!lod)
    {
        return;
    }

    const auto& mesh = lod.get();
    {
        const auto& submesh_entities = get_submesh_entities();
        auto transforms = get_transforms_for_submeshes(submesh_entities, mesh->get_subsets_count());
        set_submesh_transforms(std::move(transforms));
    }

    const auto& skin_data = mesh->get_skin_bind_data();
    // Has skinning data?
    if(skin_data.has_bones())
    {
        const auto& bone_entities = get_bone_entities();
        auto transforms = get_transforms_for_entities(bone_entities);
        set_bone_transforms(std::move(transforms));
    }
}

void model_component::on_create_component(entt::registry& r, const entt::entity e)
{
    entt::handle entity(r, e);

    auto& component = entity.get<model_component>();
    component.set_owner(entity);

    component.set_bone_transforms({});
    component.set_bone_entities({});

    component.set_submesh_transforms({});
    component.set_submesh_entities({});
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

auto model_component::casts_reflection() const -> bool
{
    return casts_reflection_;
}

void model_component::set_bone_transforms(std::vector<math::transform>&& bone_transforms)
{
    bone_transforms_ = std::move(bone_transforms);

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

void model_component::set_submesh_transforms(std::vector<math::transform>&& submesh_transforms)
{
    submesh_transforms_ = std::move(submesh_transforms);

    touch();
}

auto model_component::get_submesh_transforms() const -> const std::vector<math::transform>&
{
    return submesh_transforms_;
}

void model_component::set_submesh_entities(const std::vector<entt::handle>& submesh_entities)
{
    submesh_entities_ = submesh_entities;

    touch();
}

auto model_component::get_submesh_entities() const -> const std::vector<entt::handle>&
{
    return submesh_entities_;
}

} // namespace ace
