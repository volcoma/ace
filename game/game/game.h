#pragma once

#include <cmd_line/parser.h>
#include <context/context.hpp>

namespace ace
{

struct game
{
    static auto create(rtti::context& ctx, cmd_line::parser& parser) -> bool;
    static auto init(rtti::context& ctx, const cmd_line::parser& parser) -> bool;
    static auto deinit(rtti::context& ctx) -> bool;
    static auto destroy(rtti::context& ctx) -> bool;
    static auto process(rtti::context& ctx) -> bool;
};
} // namespace ace