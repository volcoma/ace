#pragma once
#include <engine/engine_export.h>

#include <base/basetypes.hpp>
#include <context/context.hpp>
#include <itc/thread_pool.h>
#include <itc/when_all_any.hpp>
#include <memory>

namespace ace
{

struct threader
{
    threader();
    ~threader();

    auto init(rtti::context& ctx) -> bool;
    auto deinit(rtti::context& ctx) -> bool;

    void process();

    std::unique_ptr<itc::thread_pool> pool{};
};
} // namespace ace
