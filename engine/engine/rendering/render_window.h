#pragma once

#include <graphics/render_pass.h>
#include <ospp/window.h>
#include <ospp/event.h>
#include <ospp/init.h>

#include <memory>

struct render_window
{
public:
	using graphics_surface_t = std::shared_ptr<gfx::frame_buffer>;

	render_window(os::window&& win);
	~render_window();

	void prepare_surface();
	void destroy_surface();

    void resize(uint32_t w, uint32_t h);
	auto get_window() -> os::window&;
	auto get_surface() -> graphics_surface_t&;
    auto begin_present_pass() -> gfx::render_pass&;
    auto get_present_pass() -> gfx::render_pass&;

private:
	os::window window_;
    std::unique_ptr<gfx::render_pass> pass_;
	/// Render surface for this window.
	mutable graphics_surface_t surface_;
};
