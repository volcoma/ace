#pragma once

#include <context/context.hpp>

namespace ace
{

auto get_app_ctx() -> rtti::context&;

struct meta
{
    auto init(rtti::context& ctx) -> bool;
    auto deinit(rtti::context& ctx) -> bool;

};

}
