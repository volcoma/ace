#pragma once
#include "basic_component.h"
#include <engine/rendering/model.h>

namespace ace
{
class material;

/**
 * @class model_component
 * @brief Class that contains core data for meshes.
 */
class model_component : public component_crtp<model_component, owned_component>
{
public:
    /**
     * @brief Called when the component is created.
     * @param r The registry containing the component.
     * @param e The entity associated with the component.
     */
    static void on_create_component(entt::registry& r, const entt::entity e);

    /**
     * @brief Called when the component is destroyed.
     * @param r The registry containing the component.
     * @param e The entity associated with the component.
     */
    static void on_destroy_component(entt::registry& r, const entt::entity e);

    /**
     * @brief Sets whether the model casts shadows.
     * @param cast_shadow True if the model casts shadows, false otherwise.
     */
    void set_casts_shadow(bool cast_shadow);

    /**
     * @brief Sets whether the model casts reflections.
     * @param casts_reflection True if the model casts reflections, false otherwise.
     */
    void set_casts_reflection(bool casts_reflection);

    /**
     * @brief Sets whether the model is static.
     * @param is_static True if the model is static, false otherwise.
     */
    void set_static(bool is_static);

    /**
     * @brief Checks if the model casts shadows.
     * @return True if the model casts shadows, false otherwise.
     */
    auto casts_shadow() const -> bool;

    /**
     * @brief Checks if the model casts reflections.
     * @return True if the model casts reflections, false otherwise.
     */
    auto casts_reflection() const -> bool;

    /**
     * @brief Checks if the model is static.
     * @return True if the model is static, false otherwise.
     */
    auto is_static() const -> bool;

    /**
     * @brief Gets the model.
     * @return A constant reference to the model.
     */
    auto get_model() const -> const model&;

    /**
     * @brief Sets the model.
     * @param model The model to set.
     */
    void set_model(const model& model);

    /**
     * @brief Gets the bone transforms.
     * @return A constant reference to the vector of bone transforms.
     */
    auto get_bone_transforms() const -> const pose_mat4&;

    /**
     * @brief Gets the submesh transforms.
     * @return A constant reference to the vector of submesh transforms.
     */
    auto get_submesh_transforms() const -> const pose_mat4&;

    /**
     * @brief Gets the armature entities.
     * @return A constant reference to the vector of armature entity handles.
     */
    auto get_armature_entities() const -> const std::vector<entt::handle>&;

    auto get_armature_by_id(const std::string& node_id) const -> entt::handle;
    auto get_armature_by_index(size_t index) const -> entt::handle;
    auto get_bone_by_index(size_t index) const -> entt::handle;
    auto get_skinning_transforms() const -> const std::vector<pose_mat4>&;

    /**
     * @brief Updates the armature of the model.
     */
    void update_armature();
    void create_armature();

    /**
     * @brief Sets the armature entities.
     * @param submesh_entities A vector of handles to the armature entities.
     */
    void set_armature_entities(const std::vector<entt::handle>& submesh_entities);


    /**
     * @brief Gets the local bounding box for this mesh.
     *
     * @return const math::bbox& The bounding box.
     */
    auto get_world_bounds() const -> const math::bbox&;
    void update_world_bounds(const math::transform& bounds);

    auto get_local_bounds() const -> const math::bbox&;

private:

    /**
     * @brief Sets the submesh transforms.
     * @param submesh_transforms A vector of submesh transforms.
     */
    void set_submesh_transforms(pose_mat4&& submesh_transforms);

    /**
     * @brief Sets the bone transforms.
     * @param bone_transforms A vector of bone transforms.
     */
    void set_bone_transforms(pose_mat4&& bone_transforms);

    /**
     * @brief Indicates if the model is static.
     */
    bool static_ = true;

    /**
     * @brief Indicates if the model casts shadows.
     */
    bool casts_shadow_ = true;

    /**
     * @brief Indicates if the model casts reflections.
     */
    bool casts_reflection_ = true;

    /**
     * @brief The model object.
     */
    model model_;

    /**
     * @brief Vector of handles to the armature entities.
     */
    std::vector<entt::handle> armature_entities_;

    /**
     * @brief Vector of bone transforms.
     */
    pose_mat4 bone_pose_;

    /**
     * @brief Vector of submesh transforms.
     */
    pose_mat4 submesh_pose_;

    /**
     * @brief Skinning pose per palette
     */
    std::vector<pose_mat4> skinning_pose_;


    /**
     * @brief World bounds
     */
    math::bbox world_bounds_;
};

struct bone_component : public component_crtp<bone_component>
{
    uint32_t bone_index{};
};

struct submesh_component : public component_crtp<submesh_component>
{
    std::vector<uint32_t> submeshes{};
};

} // namespace ace
