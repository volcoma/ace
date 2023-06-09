#include "renderer.h"
#include "../events.h"

#include <base/assert.hpp>
#include <graphics/debugdraw.h>
#include <graphics/graphics.h>
#include <graphics/render_pass.h>

#include <logging/logging.h>

#include <algorithm>
#include <cstdarg>

namespace ace
{
renderer::renderer(rtti::context& ctx, cmd_line::parser& parser)
{
	gfx::set_trace_logger([](const std::string& msg) { APPLOG_TRACE(msg); });
	gfx::set_info_logger([](const std::string& msg) { APPLOG_INFO(msg); });
	gfx::set_warning_logger([](const std::string& msg) { APPLOG_WARNING(msg); });
	gfx::set_error_logger([](const std::string& msg) { APPLOG_ERROR(msg); });

	auto& ev = ctx.get<events>();
	ev.on_os_event.connect(sentinel_, this, &renderer::on_os_event);
	ev.on_frame_begin.connect(sentinel_, this, &renderer::frame_begin);
	ev.on_frame_end.connect(sentinel_, this, &renderer::frame_end);

	parser.set_optional<std::string>("r", "renderer", "auto", "Select preferred renderer.");
	parser.set_optional<bool>("n", "novsync", false, "Disable vsync.");
}

auto renderer::init(const cmd_line::parser& parser) -> bool
{
	if(!os::init())
	{
		return false;
	}

	if(!init_backend(parser))
	{
		return false;
	}

	os::window window("ACE", os::window::centered, os::window::centered, 1280, 720, os::window::resizable);
//	window.request_focus();

	auto rend_win = std::make_unique<render_window>(std::move(window));

	register_window(std::move(rend_win));
	process_pending_windows();

	return true;
}

auto renderer::init_backend(const cmd_line::parser& parser) -> bool
{

	init_window_ = std::make_unique<os::window>("INIT", os::window::centered, os::window::centered, 64, 64,
												os::window::hidden);
	const auto sz = init_window_->get_size();

	gfx::init_type init_data;
	init_data.type = get_renderer_type(parser);
	init_data.resolution.width = sz.w;
	init_data.resolution.height = sz.h;
	init_data.resolution.reset = get_reset_flags(parser);
	init_data.platformData.ndt = init_window_->get_native_display();
	init_data.platformData.nwh = init_window_->get_native_handle();

	if(!gfx::init(init_data))
	{
		APPLOG_ERROR("Could not initialize rendering backend!");
		return false;
	}

	if(gfx::get_renderer_type() == gfx::renderer_type::Direct3D9)
	{
		APPLOG_ERROR("Does not support dx9. Minimum supported is dx11.");
		return false;
	}

	APPLOG_INFO("Using {0} rendering backend.", gfx::get_renderer_name(gfx::get_renderer_type()));

	if(gfx::get_renderer_type() == gfx::renderer_type::Direct3D12)
	{
		APPLOG_WARNING("Directx 12 support is experimental and unstable.");
	}

	APPLOG_INFO("DebugDraw Init.");
	ddInit();

	return true;
}

void renderer::on_os_event(rtti::context& ctx, const os::event& e)
{
	if(e.type == os::events::window)
	{
		if(e.window.type == os::window_event_id::close)
		{
			auto window_id = e.window.window_id;
			windows_.erase(std::remove_if(std::begin(windows_), std::end(windows_),
										  [window_id](const auto& window)
										  { return window->get_window().get_id() == window_id; }),
						   std::end(windows_));
		}

		if(e.window.type == os::window_event_id::resized)
		{
			auto window_id = e.window.window_id;

			auto it = std::find_if(std::begin(windows_), std::end(windows_),
								   [window_id](const auto& window)
								   { return window->get_window().get_id() == window_id; });
			if(it != std::end(windows_))
			{
				auto& win = *it;
				win->prepare_surface();
			}
		}
	}
}

auto renderer::get_renderer_type(const cmd_line::parser& parser) const -> gfx::renderer_type
{
	// auto detect
	auto preferred_renderer_type = gfx::renderer_type::Count;

	std::string preferred_renderer;
	if(parser.try_get("renderer", preferred_renderer))
	{
		if(preferred_renderer == "opengl")
		{
			preferred_renderer_type = gfx::renderer_type::OpenGL;
		}
		else if(preferred_renderer == "vulkan")
		{
			preferred_renderer_type = gfx::renderer_type::Vulkan;
		}
		else if(preferred_renderer == "directx11")
		{
			preferred_renderer_type = gfx::renderer_type::Direct3D11;
		}
		else if(preferred_renderer == "directx12")
		{
			preferred_renderer_type = gfx::renderer_type::Direct3D12;
		}
	}

	return preferred_renderer_type;
}

auto renderer::get_reset_flags(const cmd_line::parser& parser) const -> uint32_t
{
	bool novsync = false;
	parser.try_get("novsync", novsync);

	return novsync ? BGFX_RESET_NONE : BGFX_RESET_VSYNC;
}

renderer::~renderer()
{
	windows_.clear();
	windows_pending_addition_.clear();

	gfx::set_trace_logger(nullptr);
	gfx::set_info_logger(nullptr);
	gfx::set_warning_logger(nullptr);
	gfx::set_error_logger(nullptr);

	ddShutdown();
	gfx::shutdown();

	os::shutdown();
}

auto renderer::get_focused_window() const -> render_window*
{
	render_window* focused_window = nullptr;

	const auto& windows = get_windows();
	auto it = std::find_if(std::begin(windows), std::end(windows),
						   [](const auto& window) { return window->get_window().has_focus(); });

	if(it != std::end(windows))
	{
		focused_window = it->get();
	}

	return focused_window;
}

void renderer::register_window(std::unique_ptr<render_window>&& window)
{
	windows_pending_addition_.emplace_back(std::move(window));
}

auto renderer::get_windows() const -> const std::vector<std::unique_ptr<render_window>>&
{
	return windows_;
}

auto renderer::get_window(uint32_t id) const -> const std::unique_ptr<render_window>&
{
	auto it = std::find_if(std::begin(windows_), std::end(windows_),
						   [id](const auto& window) { return window->get_window().get_id() == id; });

	ensures(it != std::end(windows_));

	return *it;
}

auto renderer::get_main_window() const -> const std::unique_ptr<render_window>&
{
	expects(!windows_.empty());

	return windows_.front();
}

void renderer::hide_all_secondary_windows()
{
	for(auto& window : windows_)
	{
		bool is_main_window = window->get_window().get_id() == get_main_window()->get_window().get_id();
		if(!is_main_window)
		{
			window->get_window().hide();
		}
	}
}

void renderer::show_all_secondary_windows()
{
	for(auto& window : windows_)
	{
		bool is_main_window = window->get_window().get_id() == get_main_window()->get_window().get_id();
		if(!is_main_window)
		{
			window->get_window().show();
		}
	}
}

void renderer::process_pending_windows()
{
	std::move(std::begin(windows_pending_addition_), std::end(windows_pending_addition_),
			  std::back_inserter(windows_));
	windows_pending_addition_.clear();
}

void renderer::frame_begin(rtti::context& /*ctx*/, delta_t /*dt*/)
{
	process_pending_windows();

    auto& window = get_main_window();
    auto& pass = window->begin_present_pass();
    pass.clear();

    {
        DebugDrawEncoder encoder;

        encoder.begin(pass.id);

        {
            DebugDrawEncoderScopePush dd(encoder);
            encoder.lineTo(0, 0);
            encoder.lineTo(500, 500);
            encoder.close();
        }
        encoder.end();
    }
}

void renderer::frame_end(rtti::context& /*ctx*/, delta_t /*dt*/)
{

	gfx::render_pass pass("backbuffer_update");
	pass.bind();
	pass.clear();

	render_frame_ = gfx::frame();

	gfx::render_pass::reset();
}

auto renderer::get_render_frame() const -> uint32_t
{
	return render_frame_;
}
} // namespace ace
