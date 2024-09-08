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
                       std::vector<entt::handle>& nodes) -> entt::handle
{
    auto entity_node = parent;

    if(entity_node == parent)
    {
        auto& reg = *entity_node.registry();
        entity_node = scene::create_entity(reg, node->name, parent);
        auto& trans_comp = entity_node.get<transform_component>();
        trans_comp.set_transform_local(node->local_transform);

        nodes.push_back(entity_node);

        if(!node->submeshes.empty())
        {
            auto& comp = entity_node.emplace<submesh_component>();
            comp.submeshes = node->submeshes;
            // submesh_nodes.push_back(entity_node);
        }

        auto bone = bind_data.find_bone_by_id(node->name);
        if(bone)
        {
            entity_node.emplace<bone_component>();
            // bone_nodes.push_back(entity_node);
        }
    }

    return entity_node;
}

void process_node(const std::unique_ptr<mesh::armature_node>& node,
                  const skin_bind_data& bind_data,
                  entt::handle parent,
                  std::vector<entt::handle>& nodes)
{
    if(!parent)
        return;

    auto entity_node = process_node_impl(node, bind_data, parent, nodes);
    for(auto& child : node->children)
    {
        process_node(child, bind_data, entity_node, nodes);
    }
}

auto process_armature(const mesh& render_mesh, entt::handle parent, std::vector<entt::handle>& nodes) -> bool
{
    const auto& root = render_mesh.get_armature();
    if(!root)
    {
        return false;
    }

    const auto& skin_data = render_mesh.get_skin_bind_data();
    process_node(root, skin_data, parent, nodes);

    return true;
}

struct current_pose
{
    pose_mat4 submesh_pose;
    pose_mat4 bone_pose;
    pose_mat4 armature_pose;
};

auto get_transforms_for_entities(const std::vector<entt::handle>& entities, size_t bone_count, size_t submesh_count)
    -> current_pose
{
    size_t entities_count = entities.size();
    current_pose pose;

    pose.armature_pose.transforms.reserve(entities_count);
    pose.bone_pose.transforms.reserve(bone_count);
    pose.submesh_pose.transforms.reserve(submesh_count);

    for(const auto& e : entities)
    {
        if(e)
        {
            const auto& transform_comp = e.get<transform_component>();
            {
                const auto& transform_global = transform_comp.get_transform_global();

                pose.armature_pose.transforms.emplace_back(transform_global.get_matrix());

                if(e.all_of<bone_component>())
                {
                    pose.bone_pose.transforms.emplace_back(transform_global.get_matrix());
                }

                if(e.all_of<submesh_component>())
                {
                    pose.submesh_pose.transforms.emplace_back(transform_global.get_matrix());
                }
            }
        }
    }

    return pose;
}

auto get_transforms_for_submeshes(const std::vector<entt::handle>& entities, uint32_t submeshes_count)
    -> std::vector<math::transform>
{
    std::vector<math::transform> result;
    if(!entities.empty())
    {
        result.resize(submeshes_count, math::transform::identity());

        for(size_t i = 0; i < entities.size(); ++i)
        {
            const auto& e = entities[i];
            if(e)
            {
                const auto submesh_comp = e.try_get<submesh_component>();

                const auto transform_comp = e.try_get<transform_component>();
                if(submesh_comp && transform_comp)
                {
                    for(const auto& submesh_id : submesh_comp->submeshes)
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
    bool has_processed_armature = !get_armature_entities().empty();

    if(!has_processed_armature)
    {
        auto lod = model_.get_lod(0);
        if(!lod)
        {
            return;
        }
        const auto& mesh = lod.get();

        auto owner = get_owner();

        std::vector<entt::handle> armature_entities;
        if(process_armature(*mesh, owner, armature_entities))
        {
            set_armature_entities(armature_entities);

            const auto& skin_data = mesh->get_skin_bind_data();
            // Has skinning data?
            if(skin_data.has_bones())
            {
                set_static(false);
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

    const auto& armature_entities = get_armature_entities();
    const auto& skin_data = mesh->get_skin_bind_data();

    auto pose =
        get_transforms_for_entities(armature_entities, skin_data.get_bones().size(), mesh->get_submeshes_count());
    {
        set_submesh_transforms(std::move(pose.submesh_pose));
    }

    // Has skinning data?
    if(skin_data.has_bones())
    {
        const auto& palettes = mesh->get_bone_palettes();
        skinning_pose_.resize(palettes.size());
        for(size_t i = 0; i < palettes.size(); ++i)
        {
            const auto& palette = palettes[i];
            // Apply the bone palette.
            const auto& skinning_matrices = palette.get_skinning_matrices(pose.bone_pose.transforms, skin_data, false);
            skinning_pose_[i].transforms = skinning_matrices;
        }

        set_bone_transforms(std::move(pose.bone_pose));
    }
}

void model_component::update_world_bounds(const math::transform& world_transform)
{
    auto lod = model_.get_lod(0);
    if(!lod)
    {
        return;
    }

    const auto mesh = lod.get();
    if(mesh)
    {
        const auto& bounds = mesh->get_bounds();

        world_bounds_ = math::bbox::mul(bounds, world_transform);
    }
}

auto model_component::get_world_bounds() const -> const math::bbox&
{
    return world_bounds_;
}

auto model_component::get_local_bounds() const -> const math::bbox&
{
    auto lod = model_.get_lod(0);
    if(!lod)
    {
        return math::bbox::empty;
    }

    const auto mesh = lod.get();
    if(mesh)
    {
        return mesh->get_bounds();
    }

    return math::bbox::empty;
}

void model_component::on_create_component(entt::registry& r, const entt::entity e)
{
    entt::handle entity(r, e);

    auto& component = entity.get<model_component>();
    component.set_owner(entity);

    component.set_armature_entities({});
    component.set_bone_transforms({});
    component.set_submesh_transforms({});
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

void model_component::set_bone_transforms(pose_mat4&& bone_transforms)
{
    bone_pose_ = std::move(bone_transforms);

    touch();
}

auto model_component::get_bone_transforms() const -> const pose_mat4&
{
    return bone_pose_;
}

auto model_component::get_skinning_transforms() const -> const std::vector<pose_mat4>&
{
    return skinning_pose_;
}

void model_component::set_submesh_transforms(pose_mat4&& submesh_transforms)
{
    submesh_pose_ = std::move(submesh_transforms);

    touch();
}

auto model_component::get_submesh_transforms() const -> const pose_mat4&
{
    return submesh_pose_;
}

void model_component::set_armature_entities(const std::vector<entt::handle>& entities)
{
    armature_entities_ = entities;

    touch();
}

auto model_component::get_armature_entities() const -> const std::vector<entt::handle>&
{
    return armature_entities_;
}

auto model_component::get_armature_by_id(const std::string& node_id) const -> entt::handle
{
    for(const auto& e : armature_entities_)
    {
        const auto& tag_comp = e.get<tag_component>();
        if(tag_comp.tag == node_id)
        {
            return e;
        }
    }

    return {};
}

auto model_component::get_armature_by_index(size_t bone_index) const -> entt::handle
{
    if(bone_index >= armature_entities_.size())
    {
        return {};
    }

    return armature_entities_[bone_index];
}

} // namespace ace
