#pragma once

#include <base/basetypes.hpp>
#include <context/context.hpp>
#include <reflection/reflection.h>

namespace ace
{
class inspector_panel
{
public:

    void init(rtti::context& ctx);
    void deinit(rtti::context& ctx);

    void on_frame_ui_render(rtti::context& ctx);

private:
    rttr::variant locked_object_;
};
} // namespace ace
