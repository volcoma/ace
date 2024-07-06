#pragma once
#include "scene.h"

namespace ace
{

/**
 * @struct ecs
 * @brief Manages the entity-component-system (ECS) operations for the ACE framework.
 */
struct ecs
{
    /**
     * @brief Initializes the ECS with the given context.
     * @param ctx The context to initialize with.
     * @return True if initialization was successful, false otherwise.
     */
    auto init(rtti::context& ctx) -> bool;

    /**
     * @brief Deinitializes the ECS with the given context.
     * @param ctx The context to deinitialize.
     * @return True if deinitialization was successful, false otherwise.
     */
    auto deinit(rtti::context& ctx) -> bool;

    /**
     * @brief Renders a frame.
     * @param ctx The context for rendering.
     * @param dt The delta time for the frame.
     */
    void on_frame_render(rtti::context& ctx, delta_t dt);

    /**
     * @brief Unloads the current scene.
     */
    void unload_scene();

    /**
     * @brief Gets the current scene.
     * @return A reference to the current scene.
     */
    auto get_scene() -> scene&;

    /**
     * @brief Gets the current scene (const version).
     * @return A constant reference to the current scene.
     */
    auto get_scene() const -> const scene&;

private:
    /**
     * @brief The scene managed by the ECS.
     */
    scene scene_{};

    /**
     * @brief Sentinel value to manage shared resources.
     */
    std::shared_ptr<int> sentinel_ = std::make_shared<int>(0);
};

} // namespace ace
