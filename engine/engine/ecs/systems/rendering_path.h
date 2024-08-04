#pragma once

#include <engine/ecs/ecs.h>
#include <engine/ecs/components/camera_component.h>
#include <graphics/frame_buffer.h>
#include <graphics/render_view.h>

#include <base/basetypes.hpp>
#include <context/context.hpp>

#include <map>
#include <memory>
#include <vector>

namespace ace
{

/**
 * @class rendering_path
 * @brief Base class for different rendering paths in the ACE framework.
 */
class rendering_path
{
public:
    rendering_path() = default;
    ~rendering_path() = default;

    /**
     * @brief Initializes the rendering path with the given context.
     * @param ctx The context to initialize with.
     * @return True if initialization was successful, false otherwise.
     */
    auto init(rtti::context& ctx) -> bool;

    /**
     * @brief Deinitializes the rendering path with the given context.
     * @param ctx The context to deinitialize.
     * @return True if deinitialization was successful, false otherwise.
     */
    auto deinit(rtti::context& ctx) -> bool;

    /**
     * @brief Prepares the scene for rendering.
     * @param scn The scene to prepare.
     * @param dt The delta time.
     */
    void prepare_scene(scene& scn, delta_t dt);

    /**
     * @brief Renders the scene and returns the frame buffer.
     * @param scn The scene to render.
     * @param dt The delta time.
     * @return A shared pointer to the frame buffer containing the rendered scene.
     */
    auto render_scene(scene& scn, delta_t dt) -> gfx::frame_buffer::ptr;

    /**
     * @brief Renders the scene to the specified output.
     * @param output The output frame buffer.
     * @param scn The scene to render.
     * @param dt The delta time.
     */
    void render_scene(const gfx::frame_buffer::ptr& output, scene& scn, delta_t dt);


    /**
     * @brief Renders the scene and returns the frame buffer.
     * @param scn The scene to render.
     * @param dt The delta time.
     * @return A shared pointer to the frame buffer containing the rendered scene.
     */
    auto render_scene(camera_component& comp, scene& scn, delta_t dt) -> gfx::frame_buffer::ptr;

    /**
     * @brief Renders the scene to the specified output.
     * @param output The output frame buffer.
     * @param scn The scene to render.
     * @param dt The delta time.
     */
    void render_scene(const gfx::frame_buffer::ptr& output, camera_component& comp, scene& scn, delta_t dt);


};

} // namespace ace
