#include "imgui_interface.h"
#include <imgui/imgui_internal.h>

#include <runtime/events.h>
#include <runtime/rendering/renderer.h>

namespace ace
{

imgui_interface::imgui_interface(rtti::context& ctx)
{
	auto& ev = ctx.get<events>();

	ev.on_os_event.connect(sentinel_, 1000, this, &imgui_interface::on_os_event);
	ev.on_frame_ui_render.connect(sentinel_, this, &imgui_interface::on_frame_ui_render);

    panels_ = std::make_unique<imgui_panels>();
}

imgui_interface::~imgui_interface()
{
    panels_.reset();
	imguiDestroy();
}

void imgui_interface::init(rtti::context& ctx)
{
    const auto& rend = ctx.get<renderer>();
	const auto& main_window = rend.get_main_window();
	imguiCreate(main_window.get(), 24.0f);

}

void imgui_interface::on_os_event(rtti::context& ctx, const os::event& e)
{
    imguiProcessEvent(e);
}

void imgui_interface::on_frame_ui_render(rtti::context& ctx, delta_t dt)
{
	const auto& rend = ctx.get<renderer>();
	const auto& main_window = rend.get_main_window();
	const auto& main_surface = main_window->get_surface();


	imguiBeginFrame(dt.count());

    panels_->draw(ctx);

    gfx::render_pass pass("imgui_pass");
	pass.bind(main_surface.get());
	imguiEndFrame(pass.id);
}

} // namespace ace
