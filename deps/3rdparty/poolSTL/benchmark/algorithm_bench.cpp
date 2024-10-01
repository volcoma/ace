// Copyright (C) 2023 Adam Lugowski. All rights reserved.
// Use of this source code is governed by:
// the BSD 2-clause license, the MIT license, or at your choosing the BSL-1.0 license found in the LICENSE.*.txt files.
// SPDX-License-Identifier: BSD-2-Clause OR MIT OR BSL-1.0


#include <algorithm>
#include <vector>

#include <benchmark/benchmark.h>

#include "utils.hpp"
#include <poolstl/algorithm>
#include "../tests/thirdparty/pdqsort.h"
#include "../tests/inplace_merge_without_buffer.hpp"

////////////////////////////////

template <class ExecPolicy>
void all_of(benchmark::State& state) {
    auto values = iota_vector(arr_length);

    for ([[maybe_unused]] auto _ : state) {
        if constexpr (is_policy<ExecPolicy>::value) {
            auto res = std::all_of(policy<ExecPolicy>::get(), values.begin(), values.end(), [&](auto v) { return v >= 0; });
            benchmark::DoNotOptimize(res);
        } else {
            auto res = std::all_of(values.begin(), values.end(), [&](auto v) { return v >= 0; });
            benchmark::DoNotOptimize(res);
        }
        benchmark::ClobberMemory();
    }
}

BENCHMARK(all_of<seq>)->Name("all_of()")->UseRealTime();
BENCHMARK(all_of<poolstl_par>)->Name("all_of(poolstl::par)")->UseRealTime();
#ifdef POOLSTL_BENCH_STD_PAR
BENCHMARK(all_of<std_par>)->Name("all_of(std::execution::par)")->UseRealTime();
#endif

////////////////////////////////

template <class ExecPolicy>
void find_if(benchmark::State& state) {
    auto values = iota_vector<int>(arr_length);
    std::vector<int> haystack(arr_length);

    double needle_frac = ((double)state.range(0)) / 100;
    int needle_val = (int)((double)values.size() * needle_frac);
    auto needle = [&needle_val](auto test) { return test >= needle_val; };

    for ([[maybe_unused]] auto _ : state) {
        if constexpr (is_policy<ExecPolicy>::value) {
            auto res = std::find_if(policy<ExecPolicy>::get(), values.begin(), values.end(), needle);
            benchmark::DoNotOptimize(res);
        } else {
            auto res = std::find_if(values.begin(), values.end(), needle);
            benchmark::DoNotOptimize(res);
        }
        benchmark::ClobberMemory();
    }
}

BENCHMARK(find_if<seq>)->Name("find_if()")->UseRealTime()->ArgName("needle_percentile")->Arg(5)->Arg(50)->Arg(100);
BENCHMARK(find_if<poolstl_par>)->Name("find_if(poolstl::par)")->UseRealTime()->ArgName("needle_percentile")->Arg(5)->Arg(50)->Arg(100);
#ifdef POOLSTL_BENCH_STD_PAR
BENCHMARK(find_if<std_par>)->Name("find_if(std::execution::par)")->UseRealTime()->ArgName("needle_percentile")->Arg(5)->Arg(50)->Arg(100);
#endif

////////////////////////////////

template <class ExecPolicy>
void for_each(benchmark::State& state) {
    auto values = iota_vector<int>(arr_length);
    std::vector<int> dest(arr_length);

    for ([[maybe_unused]] auto _ : state) {
        if constexpr (is_policy<ExecPolicy>::value) {
            std::for_each(policy<ExecPolicy>::get(), values.begin(), values.end(), [&](auto v) { slow(); dest[v] = v; });
        } else {
            std::for_each(values.begin(), values.end(), [&](auto v) {slow(); dest[v] = v;});
        }
        benchmark::DoNotOptimize(dest);
        benchmark::ClobberMemory();
    }
}

BENCHMARK(for_each<seq>)->Name("for_each()")->UseRealTime();
BENCHMARK(for_each<poolstl_par>)->Name("for_each(poolstl::par)")->UseRealTime();
#ifdef POOLSTL_BENCH_STD_PAR
BENCHMARK(for_each<std_par>)->Name("for_each(std::execution::par)")->UseRealTime();
#endif

////////////////////////////////

template <class ExecPolicy>
void partition(benchmark::State& state) {
    auto values = iota_vector<int>(arr_length);
    std::vector<int> haystack(arr_length);

    double pivot_frac = ((double)state.range(0)) / 100;
    int pivot_val = (int)((double)values.size() * pivot_frac);
    auto pred = [pivot_val] (const int& em) { return em < pivot_val; };

    for ([[maybe_unused]] auto _ : state) {
        if constexpr (is_policy<ExecPolicy>::value) {
            auto res = std::partition(policy<ExecPolicy>::get(), values.begin(), values.end(), pred);
            benchmark::DoNotOptimize(res);
        } else {
            auto res = std::partition(values.begin(), values.end(), pred);
            benchmark::DoNotOptimize(res);
        }
        benchmark::ClobberMemory();
    }
}

BENCHMARK(partition<seq>)->Name("partition()")->UseRealTime()->ArgName("pivot_percentile")->Arg(50);
BENCHMARK(partition<poolstl_par>)->Name("partition(poolstl::par)")->UseRealTime()->ArgName("pivot_percentile")->Arg(50);
#ifdef POOLSTL_BENCH_STD_PAR
BENCHMARK(partition<std_par>)->Name("partition(std::execution::par)")->UseRealTime()->ArgName("pivot_percentile")->Arg(50);
#endif

////////////////////////////////

template <class ExecPolicy>
void sort(benchmark::State& state) {
    auto source = random_vector<int>(arr_length / 10);
//    auto source = random_vector<int>(arr_length);

    for ([[maybe_unused]] auto _ : state) {
        state.PauseTiming();
        std::vector<int> values(source);
        state.ResumeTiming();

        if constexpr (is_policy<ExecPolicy>::value) {
            std::sort(policy<ExecPolicy>::get(), values.begin(), values.end());
        } else {
            std::sort(values.begin(), values.end());
        }
        benchmark::DoNotOptimize(values);
        benchmark::ClobberMemory();
    }
}

BENCHMARK(sort<seq>)->Name("sort()")->UseRealTime();
BENCHMARK(sort<poolstl_par>)->Name("sort(poolstl::par)")->UseRealTime();
#ifdef POOLSTL_BENCH_STD_PAR
BENCHMARK(sort<std_par>)->Name("sort(std::execution::par)")->UseRealTime();
#endif

////////////////////////////////

template <class ExecPolicy, int which_impl>
void pluggable_sort(benchmark::State& state) {
    auto source = random_vector<int>(arr_length / 10);
//    auto source = random_vector<int>(arr_length);

    for ([[maybe_unused]] auto _ : state) {
        state.PauseTiming();
        std::vector<int> values(source);
        state.ResumeTiming();

        if constexpr (which_impl == 1) {
            poolstl::pluggable_sort(policy<ExecPolicy>::get(), values.begin(), values.end(), pdqsort);
        } else if constexpr (which_impl == 2) {
            poolstl::pluggable_mergesort(policy<ExecPolicy>::get(), values.begin(), values.end(), pdqsort);
        } else if constexpr (which_impl == 3) {
            poolstl::pluggable_mergesort(policy<ExecPolicy>::get(), values.begin(), values.end(), pdqsort, adapted_pipm_inplace_merge);
        } else if constexpr (which_impl == 4) {
            // pluggable_sort delegates to this, so essentially same as which_impl==1
            poolstl::pluggable_quicksort(policy<ExecPolicy>::get(), values.begin(), values.end(), pdqsort);
        } else if constexpr (which_impl == 5) {
            // does not use poolSTL at all. Just for comparison against parallelized version.
            pdqsort(values.begin(), values.end());
        }

        benchmark::DoNotOptimize(values);
        benchmark::ClobberMemory();
    }
}

//BENCHMARK(pluggable_sort<poolstl_par, 5>)->Name("pdqsort()")->UseRealTime(); // does not use poolSTL at all. Just for comparison against parallelized version.
BENCHMARK(pluggable_sort<poolstl_par, 1>)->Name("pluggable_sort(poolstl::par, ..., pdqsort)")->UseRealTime(); // uses pdqsort
//BENCHMARK(pluggable_sort<poolstl_par, 2>)->Name("pluggable_mergesort(poolstl::par, ..., pdqsort)")->UseRealTime(); // uses pdqsort and std::inplace_merge (O(n) extra memory)
//BENCHMARK(pluggable_sort<poolstl_par, 3>)->Name("pluggable_mergesort(poolstl::par, ..., pdqsort, pipm_merge)")->UseRealTime(); // uses pdqsort and adapted_pipm_inplace_merge (slower, but O(1) extra memory)

////////////////////////////////

template <class ExecPolicy>
void transform(benchmark::State& state) {
    auto values = iota_vector<int>(arr_length);
    std::vector<int> dest(arr_length);

    for ([[maybe_unused]] auto _ : state) {
        if constexpr (is_policy<ExecPolicy>::value) {
            std::transform(policy<ExecPolicy>::get(), values.begin(), values.end(), dest.begin(), [&](auto v) { slow(); return v; });
        } else {
            std::transform(values.begin(), values.end(), dest.begin(), [&](auto v) { slow(); return v; });
        }
        benchmark::DoNotOptimize(dest);
        benchmark::ClobberMemory();
    }
}

BENCHMARK(transform<seq>)->Name("transform()")->UseRealTime();
BENCHMARK(transform<poolstl_par>)->Name("transform(poolstl::par)")->UseRealTime();
#ifdef POOLSTL_BENCH_STD_PAR
BENCHMARK(transform<std_par>)->Name("transform(std::execution::par)")->UseRealTime();
#endif
