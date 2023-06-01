#pragma once

#include <context/context.hpp>
#include <cmd_line/parser.h>

namespace ace
{

struct editor
{
	static auto create(rtti::context& ctx, cmd_line::parser& parser) -> bool;
    static auto init(rtti::context& ctx, const cmd_line::parser& parser) -> bool;
	static auto deinit(rtti::context& ctx) -> bool;
	static auto process(rtti::context& ctx) -> bool;
};
}
