#pragma once

#include <context/context.hpp>
#include <base/basetypes.hpp>
#include <ospp/event.h>
#include <graphics/texture.h>

#include "integration/imgui.h"
#include "panels/panel.h"


namespace ace
{

class imgui_interface
{
public:
	imgui_interface(rtti::context& ctx);

    void init(rtti::context& ctx);

	~imgui_interface();

private:
    void on_frame_ui_render(rtti::context& ctx, delta_t dt);
	void on_os_event(rtti::context& ctx, const os::event& e);

    imgui_panels panels_{};
    std::shared_ptr<int> sentinel_ = std::make_shared<int>(0);
};
} // namespace ace
