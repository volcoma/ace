#pragma once
#include <base/basetypes.hpp>
#include <context/context.hpp>

#include <functional>

namespace ace
{
class footer_panel
{
public:
    void on_frame_ui_render(rtti::context& ctx, float footerSize, const std::function<void()>& on_draw = {});
};
} // namespace ace
