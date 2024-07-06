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
     * @brief Sets the bone entities.
     * @param bone_entities A vector of handles to the bone entities.
     */
    void set_bone_entities(const std::vector<entt::handle>& bone_entities);

    /**
     * @brief Gets the bone entities.
     * @return A constant reference to the vector of bone entity handles.
     */
    auto get_bone_entities() const -> const std::vector<entt::handle>&;

    /**
     * @brief Sets the bone transforms.
     * @param bone_transforms A vector of bone transforms.
     */
    void set_bone_transforms(const std::vector<math::transform>& bone_transforms);

    /**
     * @brief Gets the bone transforms.
     * @return A constant reference to the vector of bone transforms.
     */
    auto get_bone_transforms() const -> const std::vector<math::transform>&;

    /**
     * @brief Updates the armature of the model.
     */
    void update_armature();

private:
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
     * @brief Vector of handles to the bone entities.
     */
    std::vector<entt::handle> bone_entities_;

    /**
     * @brief Vector of bone transforms.
     */
    std::vector<math::transform> bone_transforms_;
};

} // namespace ace
