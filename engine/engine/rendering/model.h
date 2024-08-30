#pragma once
#include <base/basetypes.hpp>

#include <engine/assets/asset_handle.h>

#include "gpu_program.h"
#include "material.h"
#include "mesh.h"

#include <graphics/graphics.h>
#include <math/math.h>
#include <reflection/registration.h>
#include <serialization/serialization.h>

#include <vector>

namespace ace
{

/**
 * @class model
 * @brief Structure describing a LOD group (set of meshes), LOD transitions, and their materials.
 */
class model
{
public:
    REFLECTABLE(model)
    SERIALIZABLE(model)

    /**
     * @brief Default constructor for model.
     */
    model();

    /**
     * @brief Virtual destructor for model.
     */
    virtual ~model() = default;

    /**
     * @brief Checks if the model is valid.
     * @return True if the model is valid, false otherwise.
     */
    bool is_valid() const;

    /**
     * @brief Gets the LOD (Level of Detail) mesh for the specified level.
     * @param lod The level of detail.
     * @return The asset handle for the mesh at the specified LOD.
     */
    asset_handle<mesh> get_lod(std::uint32_t lod) const;

    /**
     * @brief Sets the LOD (Level of Detail) mesh for the specified level.
     * @param mesh The mesh to set.
     * @param lod The level of detail.
     */
    void set_lod(asset_handle<mesh> mesh, std::uint32_t lod);

    /**
     * @brief Sets the material for the specified index.
     * @param material The material to set.
     * @param index The index to set the material at.
     */
    void set_material(asset_handle<material> material, std::uint32_t index);

    /**
     * @brief Gets all the LOD meshes.
     * @return A constant reference to the vector of LOD meshes.
     */
    const std::vector<asset_handle<mesh>>& get_lods() const;

    /**
     * @brief Sets the LOD meshes.
     * @param lods The vector of LOD meshes to set.
     */
    void set_lods(const std::vector<asset_handle<mesh>>& lods);

    /**
     * @brief Gets all the materials.
     * @return A constant reference to the vector of materials.
     */
    const std::vector<asset_handle<material>>& get_materials() const;

    /**
     * @brief Sets the materials.
     * @param materials The vector of materials to set.
     */
    void set_materials(const std::vector<asset_handle<material>>& materials);

    /**
     * @brief Gets the material for the specified group.
     * @param group The group index.
     * @return The asset handle for the material of the specified group.
     */
    asset_handle<material> get_material_for_group(const size_t& group) const;

    /**
     * @brief Gets the LOD limits.
     * @return A constant reference to the vector of LOD limits.
     */
    inline const std::vector<urange32_t>& get_lod_limits() const
    {
        return lod_limits_;
    }

    /**
     * @brief Sets the LOD limits.
     * @param limits The vector of LOD limits to set.
     */
    void set_lod_limits(const std::vector<urange32_t>& limits);

    /**
     * @struct submit_callbacks
     * @brief Callbacks for submitting the model for rendering.
     */
    struct submit_callbacks
    {
        /**
         * @struct params
         * @brief Parameters for the submit callbacks.
         */
        struct params
        {
            /// Indicates if the model is skinned.
            bool skinned{};
            bool preserve_state{};
        };

        /// Callback for setup begin.
        std::function<void(const params& info)> setup_begin;
        /// Callback for setting up per instance.
        std::function<void(const params& info)> setup_params_per_instance;
        /// Callback for setting up per subset.
        std::function<void(const params& info, const material&)> setup_params_per_subset;
        /// Callback for setup end.
        std::function<void(const params& info)> setup_end;
    };

    /**
     * @brief Submits the model for rendering.
     * @param world_transform The world transform of the model.
     * @param bone_transforms The bone transforms for skinned models.
     * @param lod The level of detail to render.
     * @param callbacks The submit callbacks.
     */
    void submit(const math::transform& world_transform,
                const std::vector<math::transform>& submesh_transforms,
                const std::vector<math::transform>& bone_transforms,
                unsigned int lod,
                const submit_callbacks& callbacks) const;

    /**
     * @brief Gets the default material.
     * @return A reference to the default material asset handle.
     */
    static asset_handle<material>& default_material();

    /**
     * @brief Gets the fallback material.
     * @return A reference to the fallback material asset handle.
     */
    static asset_handle<material>& fallback_material();

private:
    /**
     * @brief Recalculates the LOD limits.
     */
    void recalulate_lod_limits();

    /**
     * @brief Resizes the materials based on the mesh.
     * @param mesh The mesh to use for resizing the materials.
     */
    void resize_materials(const asset_handle<mesh>& mesh);

    /// Collection of all materials for this model.
    std::vector<asset_handle<material>> materials_;
    /// Collection of all LODs for this model.
    std::vector<asset_handle<mesh>> mesh_lods_;
    /// LOD limits for this model.
    std::vector<urange32_t> lod_limits_;

};

} // namespace ace
