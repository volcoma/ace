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

void model::submit(gfx::view_id id,
                   const math::transform& world_transform,
                   const std::vector<math::transform>& bone_transforms,
                   bool apply_cull,
                   bool depth_write,
                   bool depth_test,
                   unsigned int lod,
                   gpu_program* program,
                   gpu_program* skinned_program,
                   const std::function<void(gpu_program&)>& setup_params) const
{
    const auto lod_mesh = get_lod(lod);
    if(!lod_mesh)
    {
        return;
    }

    auto mesh = lod_mesh.get();

    auto render_subset = [this, &mesh](gfx::view_id id,
                                       std::uint32_t group_id,
                                       const std::vector<math::transform>& matrices,
                                       bool apply_cull,
                                       bool depth_write,
                                       bool depth_test,
                                       gpu_program* program,
                                       const std::function<void(gpu_program&)>& setup_params)
    {
        bool valid_program = false;
        asset_handle<material> asset = get_material_for_group(group_id);
        if(!asset)
        {
            return;
        }

        auto mat = asset.get();

        if(program != nullptr)
        {
            valid_program = program->begin();
            if(valid_program)
            {
                setup_params(*program);
            }
        }

        if(valid_program)
        {
            mat->submit(program);
            auto state = mat->get_render_states(apply_cull, depth_write, depth_test);

            if(!matrices.empty())
            {
                using mat_type = math::transform::mat4_t;
                std::vector<mat_type> mats;
                mats.reserve(matrices.size());
                for(const auto& m : matrices)
                {
                    mats.emplace_back(m.get_matrix());
                }
                gfx::set_transform(mats.data(), static_cast<std::uint16_t>(mats.size()));
            }

            gfx::set_state(state);

            mesh->bind_render_buffers_for_subset(group_id);

            gfx::submit(id, program->native_handle());
        }

        if(program != nullptr)
        {
            program->end();
        }
    };

    const auto& skin_data = mesh->get_skin_bind_data();

    // Has skinning data?
    if(skin_data.has_bones() && !bone_transforms.empty())
    {
        // Process each palette in the skin with a matching attribute.
        const auto& palettes = mesh->get_bone_palettes();
        for(const auto& palette : palettes)
        {
            // Apply the bone palette.
            auto skinning_matrices = palette.get_skinning_matrices(bone_transforms, skin_data, false);
            // auto max_blend_index = palette.get_maximum_blend_index();

            auto data_group = palette.get_data_group();
            render_subset(id,
                          data_group,
                          skinning_matrices,
                          apply_cull,
                          depth_write,
                          depth_test,
                          skinned_program,
                          setup_params);

        } // Next Palette
    }
    else
    {
        for(std::size_t i = 0; i < mesh->get_subset_count(); ++i)
        {
            render_subset(id,
                          std::uint32_t(i),
                          {world_transform},
                          apply_cull,
                          depth_write,
                          depth_test,
                          program,
                          setup_params);
        }
    }
}

void model::submit(gfx::view_id id,
                   const math::transform& world_transform,
                   const std::vector<math::transform>& bone_transforms,
                   unsigned int lod,
                   gpu_program* program,
                   gpu_program* skinned_program,
                   const std::function<void()>& setup_params) const
{
    const auto lod_mesh = get_lod(lod);
    if(!lod_mesh)
    {
        return;
    }

    auto mesh = lod_mesh.get();

    auto render_subset = [this, &mesh](gfx::view_id id,
                                       std::uint32_t group_id,
                                       const std::vector<math::transform>& matrices,
                                       gpu_program* program,
                                       const std::function<void()>& setup_params)
    {
        if(!matrices.empty())
        {
            using mat_type = math::transform::mat4_t;
            std::vector<mat_type> mats;
            mats.reserve(matrices.size());
            for(const auto& m : matrices)
            {
                mats.emplace_back(m.get_matrix());
            }
            gfx::set_transform(mats.data(), static_cast<std::uint16_t>(mats.size()));
        }

        mesh->bind_render_buffers_for_subset(group_id);

        if(setup_params)
        {
            setup_params();
        }

        gfx::submit(id, program->native_handle());
    };

    const auto& skin_data = mesh->get_skin_bind_data();

    // Has skinning data?
    if(skin_data.has_bones() && !bone_transforms.empty())
    {
        if(skinned_program->begin())
        {

            // Process each palette in the skin with a matching attribute.
            const auto& palettes = mesh->get_bone_palettes();
            for(const auto& palette : palettes)
            {
                // Apply the bone palette.
                auto skinning_matrices = palette.get_skinning_matrices(bone_transforms, skin_data, false);
                // auto max_blend_index = palette.get_maximum_blend_index();

                auto data_group = palette.get_data_group();
                render_subset(id, data_group, skinning_matrices, skinned_program, setup_params);

            } // Next Palette

            skinned_program->end();
        }
    }
    else
    {
        if(program->begin())
        {
            for(std::size_t i = 0; i < mesh->get_subset_count(); ++i)
            {
                render_subset(id, std::uint32_t(i), {world_transform}, program, setup_params);
            }
            program->end();
        }
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
    auto subsets = m->get_subset_count();
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
