#pragma once

#include <base/basetypes.hpp>
#include <cmd_line/parser.h>
#include <context/context.hpp>

namespace ace
{

struct engine
{
    static auto create(rtti::context& ctx, cmd_line::parser& parser) -> bool;
    static auto init(const cmd_line::parser& parser) -> bool;
    static auto deinit() -> bool;
    static auto destroy() -> bool;
    static auto process() -> bool;

    static auto context() -> rtti::context&;
};
} // namespace ace
