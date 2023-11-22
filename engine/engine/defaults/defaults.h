#pragma once

#include <base/basetypes.hpp>
#include <context/context.hpp>

namespace ace
{

struct defaults
{
    defaults();
    ~defaults();

    auto init(rtti::context& ctx) -> bool;
    auto deinit(rtti::context& ctx) -> bool;

    auto init_assets(rtti::context& ctx) -> bool;
};
} // namespace ace
