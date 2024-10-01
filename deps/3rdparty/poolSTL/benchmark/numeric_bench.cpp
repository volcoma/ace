// Copyright (C) 2023 Adam Lugowski. All rights reserved.
// Use of this source code is governed by:
// the BSD 2-clause license, the MIT license, or at your choosing the BSL-1.0 license found in the LICENSE.*.txt files.
// SPDX-License-Identifier: BSD-2-Clause OR MIT OR BSL-1.0


#include <numeric>
#include <vector>

#include <benchmark/benchmark.h>

#include <poolstl/numeric>

#include "utils.hpp"


////////////////////////////////

template <class ExecPolicy>
void exclusive_scan(benchmark::State& state) {
    auto values = iota_vector(arr_length);
    std::vector<int> dest(arr_length);

    for ([[maybe_unused]] auto _ : state) {
        if constexpr (is_policy<ExecPolicy>::value) {
            std::exclusive_scan(policy<ExecPolicy>::get(), values.begin(), values.end(), dest.begin(), 0);
        } else {
            std::exclusive_scan(values.begin(), values.end(), dest.begin(), 0);
        }
        benchmark::DoNotOptimize(dest);
        benchmark::ClobberMemory();
    }
}

BENCHMARK(exclusive_scan<seq>)->Name("exclusive_scan()")->UseRealTime();
BENCHMARK(exclusive_scan<poolstl_par>)->Name("exclusive_scan(poolstl::par)")->UseRealTime();
#ifdef POOLSTL_BENCH_STD_PAR
BENCHMARK(exclusive_scan<std_par>)->Name("exclusive_scan(std::execution::par)")->UseRealTime();
#endif

////////////////////////////////

template <class ExecPolicy>
void reduce(benchmark::State& state) {
    auto values = iota_vector(arr_length);

    for ([[maybe_unused]] auto _ : state) {
        int64_t sum = 0;
        if constexpr (is_policy<ExecPolicy>::value) {
            sum = std::reduce(policy<ExecPolicy>::get(), values.begin(), values.end());
        } else {
            sum = std::reduce(values.begin(), values.end());
        }
        benchmark::DoNotOptimize(sum);
        benchmark::ClobberMemory();
    }
}

BENCHMARK(reduce<seq>)->Name("reduce()")->UseRealTime();
BENCHMARK(reduce<poolstl_par>)->Name("reduce(poolstl::par)")->UseRealTime();
#ifdef POOLSTL_BENCH_STD_PAR
BENCHMARK(reduce<std_par>)->Name("reduce(std::execution::par)")->UseRealTime();
#endif

////////////////////////////////
