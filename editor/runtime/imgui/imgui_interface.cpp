#include "imgui_interface.h"

#include <runtime/events.h>
#include <runtime/rendering/renderer.h>

namespace ace
{

imgui_interface::imgui_interface(rtti::context& ctx)
{
	auto& ev = ctx.get<events>();

//	//    ev.on_os_event.connect(sentinel_, this, &renderer::on_os_event);
	ev.on_frame_ui_render.connect(sentinel_, this, &imgui_interface::on_frame_ui_render);
}

imgui_interface::~imgui_interface()
{
	imguiDestroy();
}

void imgui_interface::init(rtti::context& ctx)
{
	imguiCreate();
}

void imgui_interface::on_frame_ui_render(rtti::context& ctx, delta_t dt)
{
	const auto& rend = ctx.get<renderer>();
	const auto& main_window = rend.get_main_window();
	const auto& main_surface = main_window->get_surface();

	gfx::render_pass pass("imgui_pass");
	pass.bind(main_surface.get());
	pass.clear();

	auto mouse_pos = os::mouse::get_position(main_window->get_window());
	auto surface_size = main_surface->get_size();

	auto mouse_state = (os::mouse::is_button_pressed(os::mouse::button::left) ? IMGUI_MBUT_LEFT : 0) |
					   (os::mouse::is_button_pressed(os::mouse::button::right) ? IMGUI_MBUT_RIGHT : 0) |
					   (os::mouse::is_button_pressed(os::mouse::button::middle) ? IMGUI_MBUT_MIDDLE : 0);

	auto mouse_scroll = 0;
	imguiBeginFrame(mouse_pos.x, mouse_pos.y, mouse_state, mouse_scroll, surface_size.width,
					surface_size.height, -1, pass.id);

	ImGui::ShowDemoWindow();

	imguiEndFrame();
}
} // namespace ace
