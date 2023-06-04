#include "render_window.h"
#include <graphics/graphics.h>

render_window::render_window(os::window&& win)
	: window_(std::move(win))
{
	prepare_surface();
}

render_window::~render_window()
{
	destroy_surface();
}

void render_window::destroy_surface()
{
	// force internal handle destruction
	if(surface_)
	{
		surface_.reset();

		gfx::flush();
	}
}

void render_window::prepare_surface()
{
	auto size = window_.get_size();

    bool needs_recreate = true;
    if(surface_)
    {
        auto surf_size = surface_->get_size();
        needs_recreate = surf_size.width != size.w || surf_size.height != size.h;
    }

	surface_ = std::make_shared<gfx::frame_buffer>(
		window_.get_native_handle(), static_cast<std::uint16_t>(size.w), static_cast<std::uint16_t>(size.h));
}

auto render_window::get_window() -> os::window&
{
	return window_;
}
auto render_window::get_surface() -> graphics_surface_t&
{
	return surface_;
}

auto render_window::begin_present_pass() -> gfx::render_pass&
{
    pass_.reset();
    pass_ = std::make_unique<gfx::render_pass>("present_to_window_pass");
	pass_->bind(surface_.get());

	return *pass_;
}


auto render_window::get_present_pass() -> gfx::render_pass&
{
	return *pass_;
}
