// Copyright (C) 2023 Adam Lugowski. All rights reserved.
// Use of this source code is governed by:
// the BSD 2-clause license, the MIT license, or at your choosing the BSL-1.0 license found in the LICENSE.*.txt files.
// SPDX-License-Identifier: BSD-2-Clause OR MIT OR BSL-1.0


#include <array>
#include <numeric>
#include <random>

constexpr std::array<int, 6> test_thread_counts = {0, 1, 2, 3, 5, 10};
constexpr std::array<int, 14> test_arr_sizes = {0, 1, 2, 3, 4, 8, 9, 10, 11, 20, 23, 77, 101};
extern std::mt19937 rng;

template <typename T=int>
std::vector<T> iota_vector(size_t size, T init={}) {
    std::vector<T> ret(size);
    std::iota(ret.begin(), ret.end(), init);
    return ret;
}

template <typename Container>
void scramble(Container& c) {
    std::mt19937 g(1);
    std::shuffle(c.begin(), c.end(), g);
}

struct stable_sort_element {
    int compared = 0;
    int nc = (int)rng();

    stable_sort_element& operator++() { ++compared; return *this; }
    stable_sort_element& operator=(int i) { compared = i; nc = (int)rng(); return *this; }
    bool operator==(const stable_sort_element& other) const { return compared == other.compared; }
    bool operator<(const stable_sort_element& other) const { return compared < other.compared; }
};
