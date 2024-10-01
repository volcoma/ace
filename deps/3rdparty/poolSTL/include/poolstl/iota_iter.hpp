// Copyright (C) 2023 Adam Lugowski. All rights reserved.
// Use of this source code is governed by:
// the BSD 2-clause license, the MIT license, or at your choosing the BSL-1.0 license found in the LICENSE.*.txt files.
// SPDX-License-Identifier: BSD-2-Clause OR MIT OR BSL-1.0

#ifndef POOLSTL_IOTA_ITER_HPP
#define POOLSTL_IOTA_ITER_HPP

#include <cstddef>
#include <iterator>

namespace poolstl {

    /**
     * An iterator over the integers.
     *
     * Effectively a view on a fictional vector populated by std::iota, but without materializing anything.
     *
     * Useful to parallelize loops that are not over a container, like this:
     *
     * \code{.cpp}
     * for (int i = 0; i < 10; ++i) {
     * }
     *\endcode
     *
     * Becomes:
     * \code{.cpp}
     * std::for_each(iota_iter<int>(0), iota_iter<int>(10), [](int i) {
     * });
     * \endcode
     *
     * @tparam T A type that acts as an integer.
     */
    template<typename T>
    class iota_iter {
    public:
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = T *;
        using reference = T;
        using iterator_category = std::random_access_iterator_tag;

        iota_iter() : value{} {}
        explicit iota_iter(T rhs) : value(rhs) {}
        iota_iter(const iota_iter<T> &rhs) : value(rhs.value) {}

        iota_iter<T> &operator=(T rhs) { value = rhs; return *this; }
        iota_iter<T> &operator=(const iota_iter &rhs) { value = rhs.value; return *this; }

        reference operator*() const { return value; }
        reference operator[](difference_type rhs) const { return value + rhs; }
        // operator-> has no meaning in this application

        bool operator==(const iota_iter<T> &rhs) const { return value == rhs.value; }
        bool operator!=(const iota_iter<T> &rhs) const { return value != rhs.value; }
        bool operator<(const iota_iter<T> &rhs) const { return value < rhs.value; }
        bool operator>(const iota_iter<T> &rhs) const { return value > rhs.value; }
        bool operator<=(const iota_iter<T> &rhs) const { return value <= rhs.value; }
        bool operator>=(const iota_iter<T> &rhs) const { return value >= rhs.value; }

        iota_iter<T> &operator+=(difference_type rhs) { value += rhs; return *this; }
        iota_iter<T> &operator-=(difference_type rhs) { value -= rhs; return *this; }

        iota_iter<T> &operator++() { ++value; return *this; }
        iota_iter<T> &operator--() { --value; return *this; }
        iota_iter<T> operator++(int) { iota_iter<T> ret(value); ++value; return ret; }
        iota_iter<T> operator--(int) { iota_iter<T> ret(value); --value; return ret; }

        difference_type operator-(const iota_iter<T> &rhs) const { return value - rhs.value; }
        iota_iter<T> operator-(difference_type rhs) const { return iota_iter(value - rhs); }
        iota_iter<T> operator+(difference_type rhs) const { return iota_iter(value + rhs); }

        friend inline iota_iter<T> operator+(difference_type lhs, const iota_iter<T> &rhs) {
            return iota_iter(lhs + rhs.value);
        }

    protected:
        T value;
    };
}

namespace std {
    /**
     * Specialize std::iterator_traits for poolstl::iota_iter.
     */
    template <typename T>
    struct iterator_traits<poolstl::iota_iter<T>> {
        using value_type =        typename poolstl::iota_iter<T>::value_type;
        using difference_type =   typename poolstl::iota_iter<T>::difference_type;
        using pointer =           typename poolstl::iota_iter<T>::pointer;
        using reference =         typename poolstl::iota_iter<T>::reference;
        using iterator_category = typename poolstl::iota_iter<T>::iterator_category;
    };
}

#endif
