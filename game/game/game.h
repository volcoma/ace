#pragma once

#include <cmd_line/parser.h>
#include <context/context.hpp>

namespace ace
{

struct game
{
    static auto create(rtti::context& ctx, cmd_line::parser& parser) -> bool;
    static auto init(const cmd_line::parser& parser) -> bool;
    static auto deinit() -> bool;
    static auto destroy() -> bool;
    static auto process() -> bool;
};
} // namespace ace
