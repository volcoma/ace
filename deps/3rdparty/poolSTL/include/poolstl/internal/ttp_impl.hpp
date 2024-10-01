// Copyright (C) 2023 Adam Lugowski. All rights reserved.
// Use of this source code is governed by:
// the BSD 2-clause license, the MIT license, or at your choosing the BSL-1.0 license found in the LICENSE.*.txt files.
// SPDX-License-Identifier: BSD-2-Clause OR MIT OR BSL-1.0

#ifndef POOLSTL_INTERNAL_TTP_IMPL_HPP
#define POOLSTL_INTERNAL_TTP_IMPL_HPP

#include <algorithm>
#include <future>
#include <numeric>
#include <utility>
#include <vector>

#include "utils.hpp"
#include "../execution"

namespace poolstl {
    namespace internal {

#if POOLSTL_HAVE_CXX17_LIB
        /**
         * Call std::apply in parallel.
         */
        template <class ExecPolicy, class Op, class ArgContainer>
        std::vector<std::future<void>>
        parallel_apply(ExecPolicy &&policy, Op op, const ArgContainer& args_list) {
            std::vector<std::future<void>> futures;
            auto& task_pool = *policy.pool();

            for (const auto& args : args_list) {
                futures.emplace_back(task_pool.submit([](Op op, const auto& args_fwd) {
                        std::apply(op, args_fwd);
                    }, op, args));
            }

            return futures;
        }
#endif

        /**
         * Chunk a single range, with autodetected return types.
         */
        template <class ExecPolicy, class RandIt, class Chunk,
            class ChunkRet = decltype(std::declval<Chunk>()(std::declval<RandIt>(), std::declval<RandIt>()))>
        std::vector<std::future<ChunkRet>>
        parallel_chunk_for_gen(ExecPolicy &&policy, RandIt first, RandIt last, Chunk chunk,
                               ChunkRet* = (decltype(std::declval<Chunk>()(std::declval<RandIt>(),
                                                     std::declval<RandIt>()))*)nullptr,
                               int extra_split_factor = 1) {
            std::vector<std::future<ChunkRet>> futures;
            auto& task_pool = *policy.pool();
            auto chunk_size = get_chunk_size(first, last, extra_split_factor * task_pool.get_num_threads());

            while (first != last) {
                auto iter_chunk_size = get_iter_chunk_size(first, last, chunk_size);
                RandIt loop_end = advanced(first, iter_chunk_size);

                futures.emplace_back(task_pool.submit(chunk, first, loop_end));

                first = loop_end;
            }

            return futures;
        }

        /**
         * Chunk a single range.
         */
        template <class ExecPolicy, class RandIt, class Chunk, class ChunkRet, typename... A>
        std::vector<std::future<ChunkRet>>
        parallel_chunk_for_1(ExecPolicy &&policy, RandIt first, RandIt last,
                             Chunk chunk, ChunkRet*, int extra_split_factor, A&&... chunk_args) {
            std::vector<std::future<ChunkRet>> futures;
            auto& task_pool = *policy.pool();
            auto chunk_size = get_chunk_size(first, last, extra_split_factor * task_pool.get_num_threads());

            while (first != last) {
                auto iter_chunk_size = get_iter_chunk_size(first, last, chunk_size);
                RandIt loop_end = advanced(first, iter_chunk_size);

                futures.emplace_back(task_pool.submit(chunk, first, loop_end, chunk_args...));

                first = loop_end;
            }

            return futures;
        }

        /**
         * Chunk a single range.
         */
        template <class ExecPolicy, class RandIt, class Chunk, class ChunkRet, typename... A>
        typename std::enable_if<!is_pure_threads_policy<ExecPolicy>::value, void>::type
        parallel_chunk_for_1_wait(ExecPolicy &&policy, RandIt first, RandIt last,
                                  Chunk chunk, ChunkRet* rettype, int extra_split_factor, A&&... chunk_args) {
            auto futures = parallel_chunk_for_1(std::forward<ExecPolicy>(policy), first, last,
                                                chunk, rettype, extra_split_factor, chunk_args...);
            get_futures(futures);
        }

        /**
         * Element-wise chunk two ranges.
         */
        template <class ExecPolicy, class RandIt1, class RandIt2, class Chunk, class ChunkRet, typename... A>
        std::vector<std::future<ChunkRet>>
        parallel_chunk_for_2(ExecPolicy &&policy, RandIt1 first1, RandIt1 last1, RandIt2 first2,
                             Chunk chunk, ChunkRet*, A&&... chunk_args) {
            std::vector<std::future<ChunkRet>> futures;
            auto& task_pool = *policy.pool();
            auto chunk_size = get_chunk_size(first1, last1, task_pool.get_num_threads());

            while (first1 != last1) {
                auto iter_chunk_size = get_iter_chunk_size(first1, last1, chunk_size);
                RandIt1 loop_end = advanced(first1, iter_chunk_size);

                futures.emplace_back(task_pool.submit(chunk, first1, loop_end, first2, chunk_args...));

                first1 = loop_end;
                std::advance(first2, iter_chunk_size);
            }

            return futures;
        }

        /**
         * Element-wise chunk three ranges.
         */
        template <class ExecPolicy, class RandIt1, class RandIt2, class RandIt3,
                  class Chunk, class ChunkRet, typename... A>
        std::vector<std::future<ChunkRet>>
        parallel_chunk_for_3(ExecPolicy &&policy, RandIt1 first1, RandIt1 last1, RandIt2 first2, RandIt3 first3,
                           Chunk chunk, ChunkRet*, A&&... chunk_args) {
            std::vector<std::future<ChunkRet>> futures;
            auto& task_pool = *policy.pool();
            auto chunk_size = get_chunk_size(first1, last1, task_pool.get_num_threads());

            while (first1 != last1) {
                auto iter_chunk_size = get_iter_chunk_size(first1, last1, chunk_size);
                RandIt1 loop_end = advanced(first1, iter_chunk_size);

                futures.emplace_back(task_pool.submit(chunk, first1, loop_end, first2, first3, chunk_args...));

                first1 = loop_end;
                std::advance(first2, iter_chunk_size);
                std::advance(first3, iter_chunk_size);
            }

            return futures;
        }

        /**
         * Sort a range in parallel.
         *
         * @param sort_func Sequential sort method, like std::sort or std::stable_sort
         * @param merge_func Sequential merge method, like std::inplace_merge
         */
        template <class ExecPolicy, class RandIt, class Compare, class SortFunc, class MergeFunc>
        void parallel_mergesort(ExecPolicy &&policy, RandIt first, RandIt last,
                                Compare comp, SortFunc sort_func, MergeFunc merge_func) {
            if (first == last) {
                return;
            }

            // Sort chunks in parallel
            auto futures = parallel_chunk_for_gen(std::forward<ExecPolicy>(policy), first, last,
                             [&comp, sort_func] (RandIt chunk_first, RandIt chunk_last) {
                                 sort_func(chunk_first, chunk_last, comp);
                                 return std::make_pair(chunk_first, chunk_last);
                             });

            // Merge the sorted ranges
            using SortedRange = std::pair<RandIt, RandIt>;
            auto& task_pool = *policy.pool();
            std::vector<SortedRange> subranges;
            do {
                for (auto& future : futures) {
                    subranges.emplace_back(future.get());
                }
                futures.clear();

                for (std::size_t i = 0; i < subranges.size(); ++i) {
                    if (i + 1 < subranges.size()) {
                        // pair up and merge
                        auto& lhs = subranges[i];
                        auto& rhs = subranges[i + 1];
                        futures.emplace_back(task_pool.submit([&comp, merge_func] (RandIt chunk_first,
                                                                                   RandIt chunk_middle,
                                                                                   RandIt chunk_last) {
                            merge_func(chunk_first, chunk_middle, chunk_last, comp);
                            return std::make_pair(chunk_first, chunk_last);
                        }, lhs.first, lhs.second, rhs.second));
                        ++i;
                    } else {
                        // forward the final extra range
                        std::promise<SortedRange> p;
                        futures.emplace_back(p.get_future());
                        p.set_value(subranges[i]);
                    }
                }

                subranges.clear();
            } while (futures.size() > 1);
            futures.front().get();
        }

        /**
         * Quicksort worker function.
         */
        template <class RandIt, class Compare, class SortFunc, class PartFunc, class PivotFunc>
        void quicksort_impl(task_thread_pool::task_thread_pool* task_pool, const RandIt first, const RandIt last,
                            Compare comp, SortFunc sort_func, PartFunc part_func, PivotFunc pivot_func,
                            std::ptrdiff_t target_leaf_size,
                            std::vector<std::future<void>>* futures, std::mutex* mutex,
                            std::condition_variable* cv, int* inflight_spawns) {
            using T = typename std::iterator_traits<RandIt>::value_type;

            auto partition_size = std::distance(first, last);

            if (partition_size > target_leaf_size) {
                // partition the range
                auto mid = part_func(first, last, pivot_predicate<Compare, T>(comp, pivot_func(first, last)));

                if (mid != first && mid != last) {
                    // was able to partition the range, so recurse
                    std::lock_guard<std::mutex> guard(*mutex);
                    ++(*inflight_spawns);

                    futures->emplace_back(task_pool->submit(
                        quicksort_impl<RandIt, Compare, SortFunc, PartFunc, PivotFunc>,
                        task_pool, first, mid, comp, sort_func, part_func, pivot_func, target_leaf_size,
                        futures, mutex, cv, inflight_spawns));

                    futures->emplace_back(task_pool->submit(
                        quicksort_impl<RandIt, Compare, SortFunc, PartFunc, PivotFunc>,
                        task_pool, mid, last, comp, sort_func, part_func, pivot_func, target_leaf_size,
                        futures, mutex, cv, inflight_spawns));
                    return;
                }
            }

            // Range does not need to be subdivided (or was unable to subdivide). Run the sequential sort.
            {
                // notify main thread that partitioning may be finished
                std::lock_guard<std::mutex> guard(*mutex);
                --(*inflight_spawns);
            }
            cv->notify_one();

            sort_func(first, last, comp);
        }

        /**
         * Sort a range in parallel using quicksort.
         *
         * @param sort_func Sequential sort method, like std::sort or std::stable_sort
         * @param part_func Method that partitions a range, like std::partition or std::stable_partition
         * @param pivot_func Method that identifies the pivot
         */
        template <class ExecPolicy, class RandIt, class Compare, class SortFunc, class PartFunc, class PivotFunc>
        void parallel_quicksort(ExecPolicy &&policy, RandIt first, RandIt last,
                                Compare comp, SortFunc sort_func, PartFunc part_func, PivotFunc pivot_func) {
            if (first == last) {
                return;
            }

            auto& task_pool = *policy.pool();

            // Target partition size. Range will be recursively partitioned into partitions no bigger than this
            // size. Target approximately twice as many partitions as threads to reduce impact of uneven pivot
            // selection.
            auto num_threads = task_pool.get_num_threads();
            std::ptrdiff_t target_leaf_size = std::max((std::ptrdiff_t)(std::distance(first, last) / (num_threads * 2)),
                                                       (std::ptrdiff_t)5);

            if (num_threads == 1) {
                target_leaf_size = std::distance(first, last);
            }

            // task_thread_pool does not support creating task DAGs, so organize the code such that
            // all parallel tasks are independent. The parallel tasks can spawn additional parallel tasks, and they
            // record their "child" task's std::future into a common vector to be waited on by the main thread.
            std::mutex mutex;

            // Futures of parallel tasks. Access protected by mutex.
            std::vector<std::future<void>> futures;

            // For signaling that all partitioning has been completed and futures vector is complete. Uses mutex.
            std::condition_variable cv;

            // Number of `quicksort_impl` calls that haven't finished yet. Nonzero value means futures vector may
            // still be modified. Access protected by mutex.
            int inflight_spawns = 1;

            // Root task.
            quicksort_impl(&task_pool, first, last, comp, sort_func, part_func, pivot_func, target_leaf_size,
                           &futures, &mutex, &cv, &inflight_spawns);

            // Wait for all partitioning to finish.
            {
                std::unique_lock<std::mutex> lock(mutex);
                cv.wait(lock, [&] { return inflight_spawns == 0; });
            }

            // Wait on all the parallel tasks.
            get_futures(futures);
        }

        /**
         * Partition range according to predicate. Unstable.
         *
         * This implementation only parallelizes with p=2; will spawn and wait for only one task.
         */
        template <class RandIt, class Predicate>
        RandIt partition_p2(task_thread_pool::task_thread_pool &task_pool, RandIt first, RandIt last, Predicate pred) {
            auto range_size = std::distance(first, last);
            if (range_size < 4) {
                return std::partition(first, last, pred);
            }

            // approach should be generalizable to arbitrary p

            RandIt mid = std::next(first + range_size / 2);

            // partition left and right halves in parallel
            auto left_future = task_pool.submit(std::partition<RandIt, Predicate>, first, mid, pred);
            RandIt right_mid = std::partition(mid, last, pred);
            RandIt left_mid = left_future.get();

            // merge the two partitioned halves
            auto left_highs_size = std::distance(left_mid, mid);
            auto right_lows_size = std::distance(mid, right_mid);
            if (left_highs_size <= right_lows_size) {
                std::swap_ranges(left_mid, mid, right_mid - left_highs_size);
                return right_mid - left_highs_size;
            } else {
                std::swap_ranges(mid, right_mid, left_mid);
                return left_mid + right_lows_size;
            }
        }
    }
}

#endif
