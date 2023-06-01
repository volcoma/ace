#pragma once
#include "render_window.h"

#include <base/basetypes.hpp>
#include <cmd_line/parser.h>
#include <context/context.hpp>

#include <memory>
#include <vector>

namespace ace
{

struct renderer
{
	using render_window_t = std::unique_ptr<render_window>;

	renderer(rtti::context& ctx, cmd_line::parser& parser);
	~renderer();

	auto init(const cmd_line::parser& parser) -> bool;


	auto get_render_frame() const -> uint32_t;
	void register_window(std::unique_ptr<render_window>&& window);

	auto get_windows() const -> const std::vector<std::unique_ptr<render_window>>&;
	auto get_window(uint32_t id) const -> const std::unique_ptr<render_window>&;
	auto get_main_window() const -> const std::unique_ptr<render_window>&;
	void hide_all_secondary_windows();
	void show_all_secondary_windows();

	auto get_focused_window() const -> render_window*;
	void process_pending_windows();

protected:
	auto init_backend(const cmd_line::parser& parser) -> bool;

	void on_os_event(rtti::context& ctx, const os::event& e);
    void frame_begin(rtti::context& ctx, delta_t dt);
	void frame_end(rtti::context& ctx, delta_t dt);

    auto get_renderer_type(const cmd_line::parser& parser) const -> gfx::renderer_type;
    auto get_reset_flags(const cmd_line::parser& parser) const -> uint32_t;

	uint32_t render_frame_ = 0;

	/// engine windows
	std::unique_ptr<os::window> init_window_{};
	std::vector<std::unique_ptr<render_window>> windows_;
	std::vector<std::unique_ptr<render_window>> windows_pending_addition_;

	std::shared_ptr<int> sentinel_ = std::make_shared<int>(0);
};
} // namespace ace
