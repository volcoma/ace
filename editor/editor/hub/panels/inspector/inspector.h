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

    void draw(rtti::context& ctx);

private:
    rttr::variant locked_object_;
};
} // namespace ace
