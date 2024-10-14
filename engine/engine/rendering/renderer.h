#pragma once
#include <engine/engine_export.h>

#include "render_window.h"
#include <graphics/shader.h>

#include <base/basetypes.hpp>
#include <cmd_line/parser.h>
#include <context/context.hpp>

#include <memory>

namespace ace
{

struct renderer
{
    using render_window_t = std::unique_ptr<render_window>;

    renderer(rtti::context& ctx, cmd_line::parser& parser);
    ~renderer();

    auto init(rtti::context& ctx, const cmd_line::parser& parser) -> bool;
    auto deinit(rtti::context& ctx) -> bool;

    auto create_window_for_display(int index, const std::string& title, uint32_t flags)
        -> const std::unique_ptr<render_window>&;
    auto set_main_window(os::window&& window) -> const std::unique_ptr<render_window>&;
    auto get_main_window() const -> const std::unique_ptr<render_window>&;

    void request_screenshot(const std::string& file);

    auto get_vsync() const -> bool;
    void set_vsync(bool vsync);

protected:
    auto init_backend(const cmd_line::parser& parser) -> bool;

    void on_os_event(rtti::context& ctx, const os::event& e);
    void frame_begin(rtti::context& ctx, delta_t dt);
    void frame_end(rtti::context& ctx, delta_t dt);

    auto get_renderer_type(const cmd_line::parser& parser) const -> gfx::renderer_type;
    auto get_reset_flags(const cmd_line::parser& parser) const -> uint32_t;
    auto get_reset_flags(bool vsync) const -> uint32_t;

    uint32_t reset_flags_{};
    /// engine windows
    std::unique_ptr<os::window> init_window_{};
    std::unique_ptr<render_window> render_window_{};
    std::string request_screenshot_{};

    std::shared_ptr<int> sentinel_ = std::make_shared<int>(0);
};
} // namespace ace
