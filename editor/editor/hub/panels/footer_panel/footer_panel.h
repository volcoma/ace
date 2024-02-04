#pragma once
#include <base/basetypes.hpp>
#include <context/context.hpp>

#include <functional>

namespace ace
{
class footer_panel
{
public:
    void draw(rtti::context& ctx, float footerSize, const std::function<void()>& on_draw = {});
};
} // namespace ace
