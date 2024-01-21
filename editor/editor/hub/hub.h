#pragma once

#include <base/basetypes.hpp>
#include <context/context.hpp>
#include <ospp/event.h>

#include "panels/panel.h"

namespace ace
{

class hub
{
public:
    hub(rtti::context& ctx);
    auto init(rtti::context& ctx) -> bool;
    auto deinit(rtti::context& ctx) -> bool;
private:
    void on_frame_update(rtti::context& ctx, delta_t dt);
    void on_frame_render(rtti::context& ctx, delta_t dt);
    void on_frame_ui_render(rtti::context& ctx, delta_t dt);

    std::shared_ptr<int> sentinel_ = std::make_shared<int>(0);

    imgui_panels panels_{};
};
} // namespace ace
