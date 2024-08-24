#include "model.h"
#include "gpu_program.h"
#include "graphics/graphics.h"
#include "material.h"
#include "mesh.h"

namespace ace
{
model::model()
{
}

bool model::is_valid() const
{
    return !mesh_lods_.empty();
}

asset_handle<mesh> model::get_lod(std::uint32_t lod) const
{
    if(mesh_lods_.size() > lod)
    {
        auto lodMesh = mesh_lods_[lod];
        if(lodMesh)
        {
            return lodMesh;
        }

        for(unsigned int i = lod; i < mesh_lods_.size(); ++i)
        {
            auto lodMesh = mesh_lods_[i];
            if(lodMesh)
            {
                return lodMesh;
            }
        }
        for(unsigned int i = lod; i > 0; --i)
        {
            auto lodMesh = mesh_lods_[i];
            if(lodMesh)
            {
                return lodMesh;
            }
        }
    }
    return asset_handle<mesh>();
}

void model::set_lod(asset_handle<mesh> mesh, std::uint32_t lod)
{
    if(lod >= mesh_lods_.size())
    {
        mesh_lods_.resize(lod + 1);
        recalulate_lod_limits();
    }
    mesh_lods_[lod] = mesh;

    resize_materials(mesh);
}

void model::set_material(asset_handle<material> material, std::uint32_t index)
{
    if(index >= mesh_lods_.size())
    {
        mesh_lods_.resize(index + 1);
    }

    materials_[index] = material;
}

const std::vector<asset_handle<mesh>>& model::get_lods() const
{
    return mesh_lods_;
}

void model::set_lods(const std::vector<asset_handle<mesh>>& lods)
{
    auto sz1 = lods.size();
    auto sz2 = mesh_lods_.size();

    mesh_lods_ = lods;

    if(sz1 != sz2)
    {
        recalulate_lod_limits();
    }

    if(!mesh_lods_.empty())
    {
        auto& mesh = mesh_lods_[0];
        resize_materials(mesh);
    }
}

const std::vector<asset_handle<material>>& model::get_materials() const
{
    return materials_;
}

void model::set_materials(const std::vector<asset_handle<material>>& materials)
{
    materials_ = materials;
}

asset_handle<material> model::get_material_for_group(const size_t& group) const
{
    if(materials_.size() <= group)
    {
        return {};
    }

    return materials_[group];
}

void model::set_lod_limits(const std::vector<urange32_t>& limits)
{
    lod_limits_ = limits;
}

void model::submit(const math::transform& world_transform,
                   const std::vector<math::transform>& bone_transforms,
                   unsigned int lod,
                   const submit_callbacks& callbacks) const
{
    const auto lod_mesh = get_lod(lod);
    if(!lod_mesh)
    {
        return;
    }

    auto mesh = lod_mesh.get();

    const auto& skin_data = mesh->get_skin_bind_data();

    bool skinned = !force_static && skin_data.has_bones() && !bone_transforms.empty();

    submit_callbacks::params params;
    params.skinned = skinned;

    if(callbacks.setup_begin)
    {
        callbacks.setup_begin(params);
    }

    if(callbacks.setup_params_per_instance)
    {
        callbacks.setup_params_per_instance(params);
    }

    // Has skinning data?
    if(skinned)
    {
        auto render_subset_skinned = [this, &mesh](std::uint32_t group_id,
                                                   const std::vector<math::transform>& matrices,
                                                   submit_callbacks::params& params,
                                                   const submit_callbacks& callbacks)
        {
            auto asset = get_material_for_group(group_id);
            if(!asset)
            {
                return;
            }

            auto mat = asset.get();
            auto subsets = mesh->get_subsets(group_id);

            // if the callee doesn't obbey the preserve_state flag
            // then we should set the transforms inside per subset

            for(const auto& subset : subsets)
            {
                gfx::set_transform(matrices);

                mesh->bind_render_buffers_for_subset(subset);
                params.preserve_state = &subset != &subsets.back();
                callbacks.setup_params_per_subset(params, *mat);
            }
        };

        // Process each palette in the skin with a matching attribute.
        const auto& palettes = mesh->get_bone_palettes();

        for(const auto& palette : palettes)
        {
            // Apply the bone palette.
            auto skinning_matrices = palette.get_skinning_matrices(bone_transforms, skin_data, false);
            // auto max_blend_index = palette.get_maximum_blend_index();

            auto data_group = palette.get_data_group();

            render_subset_skinned(data_group, skinning_matrices, params, callbacks);

        } // Next Palette
    }
    else
    {
        auto render_subset = [this, &mesh](std::uint32_t group_id,
                                           const math::transform& matrix,
                                           submit_callbacks::params& params,
                                           const submit_callbacks& callbacks)
        {
            auto asset = get_material_for_group(group_id);
            if(!asset)
            {
                return;
            }

            auto mat = asset.get();

            auto subsets = mesh->get_subsets(group_id);

            // if the callee doesn't obbey the preserve_state flag
            // then we should set the transforms inside per subset

            for(const auto& subset : subsets)
            {
                gfx::set_transform(matrix);

                mesh->bind_render_buffers_for_subset(subset);
                params.preserve_state = &subset != &subsets.back();
                callbacks.setup_params_per_subset(params, *mat);
            }
        };

        for(std::size_t i = 0; i < mesh->get_data_groups_count(); ++i)
        {
            render_subset(std::uint32_t(i), world_transform, params, callbacks);
        }
    }

    if(callbacks.setup_end)
    {
        callbacks.setup_end(params);
    }
}

void model::recalulate_lod_limits()
{
    float upper_limit = 100.0f;
    lod_limits_.clear();
    lod_limits_.reserve(mesh_lods_.size());

    float initial = 0.1f;
    float step = initial / float(mesh_lods_.size());
    for(size_t i = 0; i < mesh_lods_.size(); ++i)
    {
        float lower_limit = 0.0f;

        if(mesh_lods_.size() - 1 != i)
        {
            lower_limit = upper_limit * (initial - ((i)*step));
        }

        lod_limits_.emplace_back(urange32_t(urange32_t::value_type(lower_limit), urange32_t::value_type(upper_limit)));
        upper_limit = lower_limit;
    }
}

void model::resize_materials(const asset_handle<mesh>& mesh)
{
    const auto& m = mesh.get();
    auto subsets = m->get_data_groups_count();
    if(materials_.size() != subsets)
    {
        materials_.resize(subsets, default_material());
    }
}

asset_handle<material>& model::default_material()
{
    static asset_handle<material> asset;
    return asset;
}

asset_handle<material>& model::fallback_material()
{
    static asset_handle<material> asset;
    return asset;
}
} // namespace ace
