// Copyright (C) 2023 Adam Lugowski. All rights reserved.
// Use of this source code is governed by:
// the BSD 2-clause license, the MIT license, or at your choosing the BSL-1.0 license found in the LICENSE.*.txt files.
// SPDX-License-Identifier: BSD-2-Clause OR MIT OR BSL-1.0

/**
 * Use poolSTL in a C++11 program.
 */

#include <iostream>
#include <vector>

#include <poolstl/poolstl.hpp>

int main() {
    std::vector<int> v = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

    // sequential
    std::for_each(poolstl::seq, v.cbegin(), v.cend(), [](int x) {
        std::cout << x;
    });
    std::cout << std::endl;

    // parallel
    std::for_each(poolstl::par, v.cbegin(), v.cend(), [](int x) {
        std::cout << x;
    });
    std::cout << std::endl;

    return 0;
}
