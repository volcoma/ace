#pragma once
#include <context/context.hpp>
#include <base/basetypes.hpp>
#include <itc/thread_pool.h>

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