#pragma once
#include <engine/engine_export.h>

#include <graphics/render_pass.h>
#include <ospp/display_mode.h>
#include <ospp/event.h>
#include <ospp/init.h>
#include <ospp/window.h>

#include <memory>

namespace ace
{

/**
 * @brief Struct representing a render window.
 */
struct render_window
{
public:
    using graphics_surface_t = std::shared_ptr<gfx::frame_buffer>;

    /**
     * @brief Constructs a render window with the specified OS window.
     * @param win The OS window to associate with this render window.
     */
    render_window(os::window&& win);

    /**
     * @brief Destructor for the render window.
     */
    ~render_window();

    /**
     * @brief Prepares the rendering surface.
     */
    void prepare_surface();

    /**
     * @brief Destroys the rendering surface.
     */
    void destroy_surface();

    /**
     * @brief Resizes the render window to the specified width and height.
     * @param w The new width of the window.
     * @param h The new height of the window.
     */
    void resize(uint32_t w, uint32_t h);

    /**
     * @brief Gets the associated OS window.
     * @return Reference to the OS window.
     */
    auto get_window() -> os::window&;

    /**
     * @brief Gets the rendering surface.
     * @return Reference to the rendering surface.
     */
    auto get_surface() -> graphics_surface_t&;

    /**
     * @brief Begins the present render pass.
     * @return Reference to the present render pass.
     */
    auto begin_present_pass() -> gfx::render_pass&;

    /**
     * @brief Gets the present render pass.
     * @return Reference to the present render pass.
     */
    auto get_present_pass() -> gfx::render_pass&;

private:
    /// The associated OS window.
    os::window window_;
    /// The render pass for presenting.
    std::unique_ptr<gfx::render_pass> pass_;
    /// The rendering surface for this window.
    mutable graphics_surface_t surface_;
};

} // namespace ace
