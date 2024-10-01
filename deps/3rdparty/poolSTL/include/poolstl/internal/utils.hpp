// Copyright (C) 2023 Adam Lugowski. All rights reserved.
// Use of this source code is governed by:
// the BSD 2-clause license, the MIT license, or at your choosing the BSL-1.0 license found in the LICENSE.*.txt files.
// SPDX-License-Identifier: BSD-2-Clause OR MIT OR BSL-1.0

#ifndef POOLSTL_INTERNAL_UTILS_HPP
#define POOLSTL_INTERNAL_UTILS_HPP

// Version macros.
#define POOLSTL_VERSION_MAJOR 0
#define POOLSTL_VERSION_MINOR 3
#define POOLSTL_VERSION_PATCH 6

#include <cstddef>
#include <functional>
#include <iterator>

#if __cplusplus >= 201703L || (defined(_MSVC_LANG) && _MSVC_LANG >= 201703L)
#define POOLSTL_HAVE_CXX17 1
#define POOLSTL_NO_DISCARD [[nodiscard]]
#else
#define POOLSTL_HAVE_CXX17 0
#define POOLSTL_NO_DISCARD
#endif

#if POOLSTL_HAVE_CXX17 && (!defined(_GLIBCXX_RELEASE) || _GLIBCXX_RELEASE >= 9)
#define POOLSTL_HAVE_CXX17_LIB 1
#else
#define POOLSTL_HAVE_CXX17_LIB 0
#endif

#if __cplusplus >= 201402L || (defined(_MSVC_LANG) && _MSVC_LANG >= 201402L)
#define POOLSTL_HAVE_CXX14 1
#else
#define POOLSTL_HAVE_CXX14 0
#endif

namespace poolstl {
    namespace internal {

        inline constexpr std::size_t get_chunk_size(std::size_t num_steps, unsigned int num_threads) {
            return (num_steps / num_threads) + ((num_steps % num_threads) > 0 ? 1 : 0);
        }

        template<typename Iterator>
        constexpr typename std::iterator_traits<Iterator>::difference_type
        get_chunk_size(Iterator first, Iterator last, unsigned int num_threads) {
            using diff_t = typename std::iterator_traits<Iterator>::difference_type;
            return static_cast<diff_t>(get_chunk_size((std::size_t)std::distance(first, last), num_threads));
        }

        template<typename Iterator>
        constexpr typename std::iterator_traits<Iterator>::difference_type
        get_iter_chunk_size(const Iterator& iter, const Iterator& last,
                            typename std::iterator_traits<Iterator>::difference_type chunk_size) {
            return std::min(chunk_size, std::distance(iter, last));
        }

        template<typename Iterator>
        Iterator advanced(Iterator iter, typename std::iterator_traits<Iterator>::difference_type offset) {
            Iterator ret = iter;
            std::advance(ret, offset);
            return ret;
        }

        /**
         * An iterator wrapper that calls std::future<>::get().
         * @tparam Iterator
         */
        template<typename Iterator>
        class getting_iter : public Iterator {
        public:
            using value_type = decltype((*std::declval<Iterator>()).get());
            using difference_type = typename std::iterator_traits<Iterator>::difference_type;
            using pointer = value_type*;
            using reference = value_type&;
            explicit getting_iter(Iterator iter) : iter(iter) {}

            getting_iter operator++() { ++iter; return *this; }
            getting_iter operator++(int) { getting_iter ret(*this); ++iter; return ret; }

            value_type operator*() { return (*iter).get(); }
            value_type operator[](difference_type offset) { return iter[offset].get(); }

            bool operator==(const getting_iter<Iterator> &other) const { return iter == other.iter; }
            bool operator!=(const getting_iter<Iterator> &other) const { return iter != other.iter; }

        protected:
            Iterator iter;
        };

        template<typename Iterator>
        getting_iter<Iterator> get_wrap(Iterator iter) {
            return getting_iter<Iterator>(iter);
        }

        template <class Container>
        void get_futures(Container& futures) {
            for (auto &future: futures) {
                future.get();
            }
        }

        /**
         * Identify a pivot element for quicksort. Chooses the middle element of the range.
         */
        template <typename Iterator>
        typename std::iterator_traits<Iterator>::value_type quicksort_pivot(Iterator first, Iterator last) {
            return *(std::next(first, std::distance(first, last) / 2));
        }

        /**
         * Predicate for std::partition (for quicksort)
         */
        template <class Compare, class T>
        struct pivot_predicate {
            pivot_predicate(Compare comp, const T& pivot) : comp(comp), pivot(pivot) {}

            bool operator()(const T& em) {
                return comp(em, pivot);
            }
            Compare comp;
            const T pivot;
        };

        /*
         * Some methods are only available with C++17 and up. Reimplement on older standards.
         */
#if POOLSTL_HAVE_CXX17_LIB
        namespace cpp17 = std;
#else
        namespace cpp17 {

            // std::reduce

            template<class InputIt, class Tp, class BinOp>
            Tp reduce(InputIt first, InputIt last, Tp init, BinOp b) {
                for (; first != last; ++first)
                    init = b(init, *first);
                return init;
            }

            template<class InputIt>
            typename std::iterator_traits<InputIt>::value_type reduce(InputIt first, InputIt last) {
                return reduce(first, last,
                              typename std::iterator_traits<InputIt>::value_type{},
                              std::plus<typename std::iterator_traits<InputIt>::value_type>());
            }

            // std::transform

            template<class InputIt, class OutputIt, class UnaryOperation>
            OutputIt transform(InputIt first1, InputIt last1, OutputIt d_first,
                               UnaryOperation unary_op) {
                while (first1 != last1) {
                    *d_first++ = unary_op(*first1++);
                }

                return d_first;
            }

            template<class InputIt1, class InputIt2, class OutputIt, class BinaryOperation>
            OutputIt transform(InputIt1 first1, InputIt1 last1,
                               InputIt2 first2, OutputIt d_first,
                               BinaryOperation binary_op) {
                while (first1 != last1) {
                    *d_first++ = binary_op(*first1++, *first2++);
                }

                return d_first;
            }
        }
#endif
    }
}

#endif
