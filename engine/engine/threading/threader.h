#pragma once

#include <base/basetypes.hpp>
#include <itc/thread_pool.h>

#include <memory>

namespace ace
{

struct threader
{
    threader();
    ~threader();

    void process();

    std::unique_ptr<itc::thread_pool> pool{};
};
} // namespace ace
