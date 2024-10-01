// Copyright (C) 2023 Adam Lugowski. All rights reserved.
// Use of this source code is governed by:
// the BSD 2-clause license, the MIT license, or at your choosing the BSL-1.0 license found in the LICENSE.*.txt files.
// SPDX-License-Identifier: BSD-2-Clause OR MIT OR BSL-1.0

#ifndef POOLSTL_VARIANT_POLICY_HPP
#define POOLSTL_VARIANT_POLICY_HPP

#include <algorithm>
#include <numeric>
#include <variant>

#if (__cplusplus >= 201703L || (defined(_MSVC_LANG) && _MSVC_LANG >= 201703L)) && \
    (!defined(_GLIBCXX_RELEASE) || _GLIBCXX_RELEASE >= 9)
#define POOLSTL_VARIANT_HAVE_CXX17_LIB 1
#else
#define POOLSTL_VARIANT_HAVE_CXX17_LIB 0
#endif

namespace poolstl {
    namespace execution {
        /**
         * A policy that allows selecting a policy at runtime.
         *
         * @tparam Variant std::variant<> of policy options.
         */
        template <typename Variant>
        struct variant_policy {
            explicit variant_policy(const Variant& policy): var(policy) {}
            Variant var;
        };

        namespace internal {
            // Example variant definition
            /*
            using poolstl_policy_variant = std::variant<
                poolstl::execution::parallel_policy,
                poolstl::execution::sequenced_policy>;
            */
        }
    }

    using execution::variant_policy;

    namespace internal {
        /**
         * Helper for enable_if_poolstl_variant
         */
        template <typename T> struct is_poolstl_variant_policy : std::false_type {};
        template <typename V> struct is_poolstl_variant_policy<
            ::poolstl::execution::variant_policy<V>> :std::true_type {};

        /**
         * To enable/disable variant_policy (for par_if) overload resolution
         */
        template <class ExecPolicy, class Tp>
        using enable_if_poolstl_variant =
            typename std::enable_if<
                is_poolstl_variant_policy<
                    typename std::remove_cv<typename std::remove_reference<ExecPolicy>::type>::type>::value,
                Tp>::type;
    }
}

/*
 * Dynamically choose policy from a std::variant.
 * Useful to choose between parallel and sequential policies at runtime via par_if.
 */

#define POOLSTL_DEFINE_PAR_IF_FWD_VOID(NS, FNAME)                                                         \
    template<class EP, typename...ARGS>                                                                   \
    poolstl::internal::enable_if_poolstl_variant<EP, void> FNAME(EP&& policy, ARGS&&...args) {            \
        std::visit([&](auto&& pol) { NS::FNAME(pol, std::forward<ARGS>(args)...); }, policy.var);         \
    }

#define POOLSTL_DEFINE_PAR_IF_FWD(NS, FNAME)                                                                          \
    template<class EP, typename...ARGS>                                                                               \
    auto FNAME(EP&& policy, ARGS&&...args) ->                                                                         \
                poolstl::internal::enable_if_poolstl_variant<EP, decltype(NS::FNAME(std::forward<ARGS>(args)...))> {  \
        return std::visit([&](auto&& pol) { return NS::FNAME(pol, std::forward<ARGS>(args)...); }, policy.var);       \
    }

namespace std {
    // <algorithm>

    POOLSTL_DEFINE_PAR_IF_FWD(std, all_of)
    POOLSTL_DEFINE_PAR_IF_FWD(std, any_of)
    POOLSTL_DEFINE_PAR_IF_FWD(std, none_of)

    POOLSTL_DEFINE_PAR_IF_FWD(std, count)
    POOLSTL_DEFINE_PAR_IF_FWD(std, count_if)

    POOLSTL_DEFINE_PAR_IF_FWD(std, copy)
    POOLSTL_DEFINE_PAR_IF_FWD(std, copy_n)

    POOLSTL_DEFINE_PAR_IF_FWD_VOID(std, fill)
    POOLSTL_DEFINE_PAR_IF_FWD(std, fill_n)

    POOLSTL_DEFINE_PAR_IF_FWD(std, find)
    POOLSTL_DEFINE_PAR_IF_FWD(std, find_if)
    POOLSTL_DEFINE_PAR_IF_FWD(std, find_if_not)

    POOLSTL_DEFINE_PAR_IF_FWD_VOID(std, for_each)
#if POOLSTL_VARIANT_HAVE_CXX17_LIB
    POOLSTL_DEFINE_PAR_IF_FWD(std, for_each_n)
#endif

    POOLSTL_DEFINE_PAR_IF_FWD(std, partition)
    POOLSTL_DEFINE_PAR_IF_FWD(std, transform)
    POOLSTL_DEFINE_PAR_IF_FWD(std, sort)

    // <numeric>

#if POOLSTL_VARIANT_HAVE_CXX17_LIB
    POOLSTL_DEFINE_PAR_IF_FWD(std, exclusive_scan)
    POOLSTL_DEFINE_PAR_IF_FWD(std, reduce)
    POOLSTL_DEFINE_PAR_IF_FWD(std, transform_reduce)
#endif
}

#ifdef POOLSTL_VERSION_MAJOR
namespace poolstl {
    // <poolstl/algorithm>

    POOLSTL_DEFINE_PAR_IF_FWD_VOID(poolstl, for_each_chunk)
    POOLSTL_DEFINE_PAR_IF_FWD_VOID(poolstl, pluggable_sort)
}
#endif

#endif
