#pragma once

#include <context/context.hpp>
#include <base/basetypes.hpp>

#include "integration/imgui.h"

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


    std::shared_ptr<int> sentinel_ = std::make_shared<int>(0);

};
} // namespace ace
