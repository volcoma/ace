#pragma once
#include <engine/engine_export.h>

#include <base/basetypes.hpp>
#include <context/context.hpp>
#include <engine/threading/threader.h>
#include <filesystem/filesystem.h>

#include <monort/monort.h>

namespace ace
{

struct script_glue
{
    auto init(rtti::context& ctx) -> bool;
    auto deinit(rtti::context& ctx) -> bool;
};
} // namespace ace
