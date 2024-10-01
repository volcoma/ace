// Copyright (C) 2023 Adam Lugowski. All rights reserved.
// Use of this source code is governed by:
// the BSD 2-clause license, the MIT license, or at your choosing the BSL-1.0 license found in the LICENSE.*.txt files.
// SPDX-License-Identifier: BSD-2-Clause OR MIT OR BSL-1.0

#include <iostream>
#include <string>
#include <variant>

// The <execution> header defines compiler-provided execution policies, but is not always present.
#if __cplusplus >= 201603L || (defined(_MSVC_LANG) && _MSVC_LANG >= 201603L)
#if __has_include(<execution>)
#ifndef POOLSTL_STD_SUPPLEMENT_NO_INCLUDE
// On GCC and Clang simply including <execution> requires linking with TBB.
// MinGW does not require TBB, but performance may be sequential. To keep parallel performance fallback to poolSTL.
#include <execution>
#endif
#endif
#endif

#define POOLSTL_STD_SUPPLEMENT
#include <poolstl/poolstl.hpp>

#if __has_include(<poolstl/variant_policy.hpp>)
// Test poolstl::variant_policy (independent header, not bundled with the amalgam).
#include <poolstl/variant_policy.hpp>
using std_policy_variant = std::variant<std::execution::parallel_policy, std::execution::sequenced_policy>;

/**
 * A version of poolstl::par_if that works on the std::execution policies.
 */
poolstl::variant_policy<std_policy_variant> std_par_if(bool call_par) {
    if (call_par) {
        return poolstl::variant_policy(std_policy_variant(std::execution::par));
    } else {
        return poolstl::variant_policy(std_policy_variant(std::execution::seq));
    }
}
#endif

int main() {
    if (std::is_same_v<std::execution::parallel_policy, poolstl::execution::parallel_policy>) {
        std::cout << "Using poolSTL supplement" << std::endl;
    } else {
        std::cout << "Using native par" << std::endl;
    }

    std::vector<int> v = {0, 1, 2, 3, 4, 5};
    std::for_each(std::execution::seq, v.cbegin(), v.cend(), [](int x) {
        std::cout << x;
    });
    std::cout << " std::execution::seq" << std::endl;

    std::for_each(std::execution::par, v.cbegin(), v.cend(), [](int x) {
        std::cout << x;
    });
    std::cout << " std::execution::par" << std::endl;

    std::for_each(std::execution::par_unseq, v.cbegin(), v.cend(), [](int x) {
        std::cout << x;
    });
    std::cout << " std::execution::par_unseq" << std::endl;

#if __has_include(<poolstl/variant_policy.hpp>)
    for (bool is_parallel : {true, false}) {
        std::for_each(std_par_if(is_parallel), v.cbegin(), v.cend(), [](int x) {
            std::cout << x;
        });
        std::cout << " std_par_if(" << std::to_string(is_parallel) << ")" << std::endl;
    }
#endif

    return 0;
}
