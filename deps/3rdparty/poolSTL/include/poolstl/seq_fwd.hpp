// Copyright (C) 2023 Adam Lugowski. All rights reserved.
// Use of this source code is governed by:
// the BSD 2-clause license, the MIT license, or at your choosing the BSL-1.0 license found in the LICENSE.*.txt files.
// SPDX-License-Identifier: BSD-2-Clause OR MIT OR BSL-1.0

#ifndef POOLSTL_VARIANT_POLICY_HPP
#define POOLSTL_VARIANT_POLICY_HPP

#include <algorithm>
#include <numeric>

/* INDEPENDENT FILE. DOES NOT REQUIRE REST OF POOLSTL.
 *
 * An execution policy that simply forwards to the basic non-policy (sequential) standard library functions.
 *
 * Allows writing code in the parallel style even if tooling does not support doing so.
 *
 * The sequential policy will be poolstl::seq_fwd.
 * You may override by defining POOLSTL_SEQ_FWD_POLICY to be the policy you'd prefer instead.
 */


#ifndef POOLSTL_SEQ_FWD_POLICY
namespace poolstl {
    namespace execution {
        struct seq_fwd_policy {
        };

        constexpr seq_fwd_policy seq_fwd;
    }
    using execution::seq_fwd;
}
#define POOLSTL_SEQ_FWD_POLICY poolstl::execution::seq_fwd_policy
#endif

#if (__cplusplus >= 201703L || (defined(_MSVC_LANG) && _MSVC_LANG >= 201703L)) && \
    (!defined(_GLIBCXX_RELEASE) || _GLIBCXX_RELEASE >= 9)
#define POOLSTL_SEQ_FWD_HAVE_CXX17_LIB 1
#else
#define POOLSTL_SEQ_FWD_HAVE_CXX17_LIB 0
#endif

namespace poolstl {
    namespace internal {
        /**
         * To enable/disable seq_fwd overload resolution
         */
        template <class ExecPolicy, class Tp>
        using enable_if_seq_fwd =
            typename std::enable_if<
                std::is_same<POOLSTL_SEQ_FWD_POLICY,
                    typename std::remove_cv<typename std::remove_reference<ExecPolicy>::type>::type>::value,
                Tp>::type;
    }
}

/*
 * Forward a specified policy to the native sequential (no policy) method.
 */

#define POOLSTL_DEFINE_SEQ_FWD(NS, FNAME)                                                                       \
    template<class EP, typename...ARGS>                                                                         \
    auto FNAME(EP&&, ARGS&&...args) ->                                                                          \
                poolstl::internal::enable_if_seq_fwd<EP, decltype(NS::FNAME(std::forward<ARGS>(args)...))> {    \
        return NS::FNAME(std::forward<ARGS>(args)...);                                                          \
    }

#define POOLSTL_DEFINE_SEQ_FWD_VOID(NS, FNAME)                                       \
    template<class EP, typename...ARGS>                                              \
    poolstl::internal::enable_if_seq_fwd<EP, void> FNAME(EP&&, ARGS&&... args) {     \
        NS::FNAME(std::forward<ARGS>(args)...);                                      \
    }


namespace std {
    // <algorithm>

    POOLSTL_DEFINE_SEQ_FWD(std, all_of)
    POOLSTL_DEFINE_SEQ_FWD(std, any_of)
    POOLSTL_DEFINE_SEQ_FWD(std, none_of)

    POOLSTL_DEFINE_SEQ_FWD(std, count)
    POOLSTL_DEFINE_SEQ_FWD(std, count_if)

    POOLSTL_DEFINE_SEQ_FWD(std, copy)
    POOLSTL_DEFINE_SEQ_FWD(std, copy_n)

    POOLSTL_DEFINE_SEQ_FWD_VOID(std, fill)
    POOLSTL_DEFINE_SEQ_FWD(std, fill_n)

    POOLSTL_DEFINE_SEQ_FWD(std, find)
    POOLSTL_DEFINE_SEQ_FWD(std, find_if)
    POOLSTL_DEFINE_SEQ_FWD(std, find_if_not)

    POOLSTL_DEFINE_SEQ_FWD_VOID(std, for_each)
#if POOLSTL_SEQ_FWD_HAVE_CXX17_LIB
    POOLSTL_DEFINE_SEQ_FWD(std, for_each_n)
#endif

    POOLSTL_DEFINE_SEQ_FWD(std, partition)
    POOLSTL_DEFINE_SEQ_FWD(std, transform)
    POOLSTL_DEFINE_SEQ_FWD(std, sort)

    // <numeric>

#if POOLSTL_SEQ_FWD_HAVE_CXX17_LIB
    POOLSTL_DEFINE_SEQ_FWD(std, exclusive_scan)
    POOLSTL_DEFINE_SEQ_FWD(std, reduce)
    POOLSTL_DEFINE_SEQ_FWD(std, transform_reduce)
#endif
}

#ifdef POOLSTL_VERSION_MAJOR
namespace poolstl {
    // <poolstl/algorithm>

    POOLSTL_DEFINE_SEQ_FWD_VOID(poolstl, for_each_chunk)
}
#endif

#endif
