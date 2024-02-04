#pragma once

#include <base/basetypes.hpp>
#include <context/context.hpp>

namespace ace
{
class header_panel
{
public:
    void draw(rtti::context& ctx, float headerSize);
};
} // namespace ace
