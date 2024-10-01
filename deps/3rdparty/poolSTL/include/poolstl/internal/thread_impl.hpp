// Copyright (C) 2023 Adam Lugowski. All rights reserved.
// Use of this source code is governed by:
// the BSD 2-clause license, the MIT license, or at your choosing the BSL-1.0 license found in the LICENSE.*.txt files.
// SPDX-License-Identifier: BSD-2-Clause OR MIT OR BSL-1.0

#ifndef POOLSTL_INTERNAL_THREAD_IMPL_HPP
#define POOLSTL_INTERNAL_THREAD_IMPL_HPP

/**
 * EXPERIMENTAL: Subject to significant changes or removal.
 * An implementation using only std::thread and no thread pool at all.
 *
 * Advantage:
 *  - Fewer symbols (no packaged_task with its operators, destructors, vtable, etc) means smaller binary
 *    which can mean a lot when there are many calls like with many templates.
 *  - No thread pool to manage.
 *
 * Disadvantages:
 *  - Threads are started and joined for every operation, so it is harder to amortize that cost.
 *  - Barely any algorithms are supported.
 */

#include <algorithm>
#include <thread>
#include <numeric>
#include <utility>
#include <vector>

#include "utils.hpp"
#include "../execution"

namespace poolstl {
    namespace internal {

        template <class ExecPolicy, class RandIt, class Chunk, class ChunkRet, typename... A>
        typename std::enable_if<is_pure_threads_policy<ExecPolicy>::value, void>::type
        parallel_chunk_for_1_wait(ExecPolicy &&policy, RandIt first, RandIt last,
                                  Chunk chunk, ChunkRet*, int extra_split_factor, A&&... chunk_args) {
            std::vector<std::thread> threads;
            auto chunk_size = get_chunk_size(first, last, extra_split_factor * policy.get_num_threads());

            while (first != last) {
                auto iter_chunk_size = get_iter_chunk_size(first, last, chunk_size);
                RandIt loop_end = advanced(first, iter_chunk_size);

                threads.emplace_back(std::thread(chunk, first, loop_end, chunk_args...));

                first = loop_end;
            }

            for (auto& thread : threads) {
                if (thread.joinable()) {
                    thread.join();
                }
            }
        }
    }
}

#endif
