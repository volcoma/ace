// Copyright (C) 2023 Adam Lugowski. All rights reserved.
// Use of this source code is governed by:
// the BSD 2-clause license, the MIT license, or at your choosing the BSL-1.0 license found in the LICENSE.*.txt files.
// SPDX-License-Identifier: BSD-2-Clause OR MIT OR BSL-1.0

#ifndef POOLSTL_INPLACE_MERGE_WITHOUT_BUFFER_HPP
#define POOLSTL_INPLACE_MERGE_WITHOUT_BUFFER_HPP

#include "thirdparty/InplaceMerge.hh"

/**
 * Like `std::inplace_merge` but uses a buffer-free algorithm described by Huang
 * and Langston in "Practical In-Place Merging", Communications of the ACM, 1988, http://dx.doi.org/10.1145/42392.42403
 * Generalizes the implementation by Keith Schwarz: http://keithschwarz.com/interesting/code/?dir=inplace-merge
 *
 * Schwarz's implementation only supports merging two ranges of equal size. This adapter also handles cases where the
 * ranges are slightly different sizes, as happens when used as the merge step for a general-purpose sort.
 * Drastically different sizes effectively fallback to std::inplace_merge, which is not buffer-free.
 */
template<class RandIt, class Compare>
void adapted_pipm_inplace_merge(RandIt first, RandIt mid, RandIt last, Compare comp) {
    auto left_size = std::distance(first, mid);
    auto right_size = std::distance(mid, last);

    if (left_size == right_size) {
        InplaceMerge(first, last, comp);
    } else if (left_size < right_size) {
        auto extra = right_size - left_size;
        InplaceMerge(first, last - extra, comp);
        std::inplace_merge(first, last - extra, last, comp);
    } else {
        // left_size > right_size
        auto extra = left_size - right_size;
        InplaceMerge(first + extra, last, comp);
        std::inplace_merge(first, first + extra, last, comp);
    }
}

#endif
