#pragma once

#include <graphics/render_pass.h>
#include <ospp/window.h>
#include <ospp/event.h>

#include <memory>

struct render_window
{
public:
	using graphics_surface_t = std::shared_ptr<gfx::frame_buffer>;

	render_window(os::window&& win);
	~render_window();

	void prepare_surface();
	void destroy_surface();

	auto get_window() -> os::window&;
	auto get_surface() -> graphics_surface_t&;
    auto begin_present_pass() -> gfx::view_id;
private:
	os::window window_;

	/// Render surface for this window.
	graphics_surface_t surface_;
};
