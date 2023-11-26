#pragma once

#include <base/basetypes.hpp>
#include <context/context.hpp>
#include <ospp/event.h>

#include "integration/imgui.h"

namespace ace
{

class imgui_interface
{
public:
    imgui_interface(rtti::context& ctx);
    ~imgui_interface();

    auto init(rtti::context& ctx) -> bool;
    auto deinit(rtti::context& ctx) -> bool;

private:
    void on_frame_ui_render(rtti::context& ctx, delta_t dt);
    void on_os_event(rtti::context& ctx, const os::event& e);

    std::shared_ptr<int> sentinel_ = std::make_shared<int>(0);
};
} // namespace ace
