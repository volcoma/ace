// Copyright (C) 2023 Adam Lugowski. All rights reserved.
// Use of this source code is governed by:
// the BSD 2-clause license, the MIT license, or at your choosing the BSL-1.0 license found in the LICENSE.*.txt files.
// SPDX-License-Identifier: BSD-2-Clause OR MIT OR BSL-1.0


#include <algorithm>
#include <atomic>

#include <catch2/catch_test_macros.hpp>

#if __has_include(<poolstl/seq_fwd.hpp>)
// seq_fwd not included in the poolSTL amalgam, so skip these tests on CI steps that test the amalgam.

#include <poolstl/seq_fwd.hpp>

#include "utils.hpp"


TEST_CASE("fwd_count", "[alg][algorithm][seq_fwd]") {
    for (auto vec_size : test_arr_sizes) {
        auto haystack = iota_vector(vec_size);

        {
            int needle = 5;
            auto seq = std::count(                  haystack.cbegin(), haystack.cend(), needle);
            auto par = std::count(poolstl::seq_fwd, haystack.cbegin(), haystack.cend(), needle);
            REQUIRE(seq == par);
        }
        {
            auto pred = [&](auto x) { return x % 2 == 0; };
            auto seq = std::count_if(                  haystack.cbegin(), haystack.cend(), pred);
            auto par = std::count_if(poolstl::seq_fwd, haystack.cbegin(), haystack.cend(), pred);
            REQUIRE(seq == par);
        }
    }
}

TEST_CASE("fwd_for_each", "[alg][algorithm][seq_fwd]") {
    std::atomic<int> sum{0};
    for (auto num_iters : test_arr_sizes) {
        auto v = iota_vector(num_iters);

        for (auto which_impl : {0, 1}) {
            sum = 0;
            auto f = [&](auto) { ++sum; };
            switch (which_impl) {
                case 0:
                    std::for_each(v.cbegin(), v.cend(), f);
                    break;
                case 1:
                    std::for_each(poolstl::seq_fwd, v.cbegin(), v.cend(), f);
                    break;
                default: break;
            }
            REQUIRE(sum == num_iters);
        }
    }
}

std::mt19937 rng{1};
#endif
