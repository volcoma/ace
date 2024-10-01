[![tests](https://github.com/alugowski/poolSTL/actions/workflows/tests.yml/badge.svg)](https://github.com/alugowski/poolSTL/actions/workflows/tests.yml)
[![codecov](https://codecov.io/gh/alugowski/poolSTL/branch/main/graph/badge.svg?token=zB7yN8NwUc)](https://codecov.io/gh/alugowski/poolSTL)

# poolSTL

Light, self-contained, thread pool-based implementation of [C++17 parallel standard library algorithms](https://en.cppreference.com/w/cpp/algorithm).

C++17 introduced parallel overloads of standard library algorithms that accept an [*Execution Policy*](https://en.cppreference.com/w/cpp/algorithm/execution_policy_tag) as the first argument.
Policies specify limits on how the implementation may parallelize the algorithm, enabling methods like threads, vectorization, or even GPU.
Policies can be supplied by the compiler or by libraries like this one.

```c++
std::sort(std::execution::par, vec.begin(), vec.end());
    //    ^^^^^^^^^^^^^^^^^^^ native C++17 parallel Execution Policy      
```

Unfortunately compiler support [varies](https://en.cppreference.com/w/cpp/compiler_support/17). Quick summary of compilers' default standard libraries:

|                   |    Linux     |    macOS     |   Windows    |
|:------------------|:------------:|:------------:|:------------:|
| GCC 9+            | TBB Required | TBB Required | TBB Required |
| GCC 8-            |      ❌      |      ❌      |      ❌      |
| Clang (libc++)    |      ❌      |      ❌      |      ❌      |
| Clang (libstdc++) | TBB Required | TBB Required | TBB Required |
| Apple Clang       |              |      ❌      |              |
| MSVC 15.7+ (2017) |              |              |      ✅      |
| [Parallel STL](https://www.intel.com/content/www/us/en/developer/articles/guide/get-started-with-parallel-stl.html) | TBB Required | TBB Required | TBB Required |
| **poolSTL**       |      ✅*     |      ✅*     |      ✅*     |

PoolSTL is a *supplement* to fill in the support gaps. It is not a full implementation; only the basics are covered.
However, it is small, easy to integrate, and has no external dependencies. A good backup to the other options.

Use poolSTL exclusively, or only on platforms lacking native support,
or only if [TBB](https://www.intel.com/content/www/us/en/developer/tools/oneapi/onetbb.html) is not present.

Supports C++11 and higher. Algorithms introduced in C++17 require C++17 or higher.  
Tested in CI on GCC 7+, Clang/LLVM 5+, Apple Clang, MSVC, MinGW, and Emscripten.

## Implemented Algorithms
Algorithms are added on an as-needed basis. If you need one [open an issue](https://github.com/alugowski/poolSTL/issues) or contribute a PR.  
**Limitations:** All iterators must be random access. No nested parallel calls.

### `<algorithm>`
* [`all_of`](https://en.cppreference.com/w/cpp/algorithm/all_of), [`any_of`](https://en.cppreference.com/w/cpp/algorithm/any_of), [`none_of`](https://en.cppreference.com/w/cpp/algorithm/none_of)
* [`copy`](https://en.cppreference.com/w/cpp/algorithm/copy), [`copy_n`](https://en.cppreference.com/w/cpp/algorithm/copy_n)
* [`count`](https://en.cppreference.com/w/cpp/algorithm/count), [`count_if`](https://en.cppreference.com/w/cpp/algorithm/count_if)
* [`fill`](https://en.cppreference.com/w/cpp/algorithm/fill), [`fill_n`](https://en.cppreference.com/w/cpp/algorithm/fill_n)
* [`find`](https://en.cppreference.com/w/cpp/algorithm/find), [`find_if`](https://en.cppreference.com/w/cpp/algorithm/find_if), [`find_if_not`](https://en.cppreference.com/w/cpp/algorithm/find_if_not)
* [`for_each`](https://en.cppreference.com/w/cpp/algorithm/for_each), [`for_each_n`](https://en.cppreference.com/w/cpp/algorithm/for_each_n)
* [`partition`](https://en.cppreference.com/w/cpp/algorithm/partition)
* [`sort`](https://en.cppreference.com/w/cpp/algorithm/sort), [`stable_sort`](https://en.cppreference.com/w/cpp/algorithm/stable_sort)
* [`transform`](https://en.cppreference.com/w/cpp/algorithm/transform)

### `<numeric>`
* [`exclusive_scan`](https://en.cppreference.com/w/cpp/algorithm/exclusive_scan) (C++17 only)
* [`reduce`](https://en.cppreference.com/w/cpp/algorithm/reduce)
* [`transform_reduce`](https://en.cppreference.com/w/cpp/algorithm/transform_reduce) (C++17 only)

All in `std::` namespace.

### Other
* [`poolstl::iota_iter`](include/poolstl/iota_iter.hpp) - Iterate over integers. Same as iterating over output of [`std::iota`](https://en.cppreference.com/w/cpp/algorithm/iota) but without materializing anything. Iterator version of [`std::ranges::iota_view`](https://en.cppreference.com/w/cpp/ranges/iota_view).
* `poolstl::for_each_chunk` - Like `std::for_each`, but explicitly splits the input range into chunks then exposes the chunked parallelism. A user-specified chunk constructor is called for each parallel chunk then its output is passed to each loop iteration. Useful for workloads that need an expensive workspace that can be reused between iterations, but not simultaneously by all iterations in parallel.
* `poolstl::pluggable_sort` - Like `std::sort`, but allows specification of sequential sort method. To parallelize [pdqsort](https://github.com/orlp/pdqsort): `pluggable_sort(par, v.begin(), v.end(), pdqsort)`.

## Usage

PoolSTL provides:
* `poolstl::par`: Substitute for [`std::execution::par`](https://en.cppreference.com/w/cpp/algorithm/execution_policy_tag). Parallelized using a [thread pool](https://github.com/alugowski/task-thread-pool).
* `poolstl::seq`: Substitute for `std::execution::seq`. Simply calls the regular (non-policy) overload.
* `poolstl::par_if()`: Choose parallel or sequential at runtime. See below.

In short, use `poolstl::par` to make your code parallel. Complete example:
```c++
#include <iostream>
#include <poolstl/poolstl.hpp>

int main() {
    std::vector<int> v = {0, 1, 2, 3, 4, 5};
    auto sum = std::reduce(poolstl::par, vec.cbegin(), vec.cend());
    //                     ^^^^^^^^^^^^
    //                     Add this to make your code parallel.
    std::cout << "Sum=" << sum << std::endl;
    return 0;
}
```

### Controlling Thread Pool Size with `par.on(pool)`

The thread pool used by `poolstl::par` is managed internally by poolSTL. It is started on first use.  
Use your own [thread pool](https://github.com/alugowski/task-thread-pool)
with `poolstl::par.on(pool)` for control over thread count, startup/shutdown, etc.:

```c++
task_thread_pool::task_thread_pool pool{4};  // 4 threads

std::reduce(poolstl::par.on(pool), vec.begin(), vec.end());
```

### Choosing Parallel or Sequential at Runtime with `par_if`

Sometimes the choice whether to parallelize or not should be made at runtime. For example, small datasets may not amortize
the cost of starting threads, while large datasets do and should be parallelized.

Use `poolstl::par_if` to select between `par` and `seq` at runtime:
```c++
bool is_parallel = vec.size() > 10000;

std::reduce(poolstl::par_if(is_parallel), vec.begin(), vec.end());
```

Use `poolstl::par_if(is_parallel, pool)` to control the thread pool used by `par`, if selected.

# Examples

### Parallel `for (auto& value : vec)`

```c++
std::vector<int> vec = {0, 1, 2, 3, 4, 5};

// Parallel for-each
std::for_each(poolstl::par, vec.begin(), vec.end(), [](auto& value) {
    std::cout << value;  // loop body
});
```

### Parallel `for (int i = 0; i < 100; ++i)`

```c++
using poolstl::iota_iter;

// parallel for loop
std::for_each(poolstl::par, iota_iter<int>(0), iota_iter<int>(100), [](auto i) {
    std::cout << i;  // loop body
});
```



### Parallel Sort

```c++
std::vector<int> vec = {5, 2, 1, 3, 0, 4};

std::sort(poolstl::par, vec.begin(), vec.end());
```

# Installation

### Single File

Each [release](https://github.com/alugowski/poolSTL/releases/latest) publishes a single-file amalgamated `poolstl.hpp`. Simply copy this into your project.

**Build requirements:**
 - Clang and GCC 8 or older: require `-lpthread` to use C++11 threads.
 - Emscripten: compile and link with `-pthread` to use C++11 threads. [See docs](https://emscripten.org/docs/porting/pthreads.html).

### CMake

```cmake
include(FetchContent)
FetchContent_Declare(
        poolSTL
        GIT_REPOSITORY https://github.com/alugowski/poolSTL
        GIT_TAG main
        GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(poolSTL)

target_link_libraries(YOUR_TARGET poolSTL::poolSTL)
```

Alternatively copy or checkout the repo into your project and:
```cmake
add_subdirectory(poolSTL)
```

# Benchmark

See [benchmark/](benchmark) to compare poolSTL against the standard sequential implementation, and (if available) the
native `std::execution::par` implementation.

Results on an M1 Pro (6 power, 2 efficiency cores), with GCC 13:
```
-------------------------------------------------------------------------------------------------------
Benchmark                                                             Time             CPU   Iterations
-------------------------------------------------------------------------------------------------------
all_of()/real_time                                                 19.9 ms         19.9 ms           35
all_of(poolstl::par)/real_time                                     3.47 ms        0.119 ms          198
all_of(std::execution::par)/real_time                              3.45 ms         3.25 ms          213
find_if()/needle_percentile:5/real_time                           0.988 ms        0.987 ms          712
find_if()/needle_percentile:50/real_time                           9.87 ms         9.86 ms           71
find_if()/needle_percentile:100/real_time                          19.7 ms         19.7 ms           36
find_if(poolstl::par)/needle_percentile:5/real_time               0.405 ms        0.050 ms         1730
find_if(poolstl::par)/needle_percentile:50/real_time               1.85 ms        0.096 ms          393
find_if(poolstl::par)/needle_percentile:100/real_time              3.64 ms        0.102 ms          193
find_if(std::execution::par)/needle_percentile:5/real_time        0.230 ms        0.220 ms         3103
find_if(std::execution::par)/needle_percentile:50/real_time        1.75 ms         1.60 ms          410
find_if(std::execution::par)/needle_percentile:100/real_time       3.51 ms         3.24 ms          204
for_each()/real_time                                               94.6 ms         94.6 ms            7
for_each(poolstl::par)/real_time                                   18.7 ms        0.044 ms           36
for_each(std::execution::par)/real_time                            15.3 ms         12.9 ms           46
sort()/real_time                                                    603 ms          602 ms            1
sort(poolstl::par)/real_time                                        112 ms         6.64 ms            6
sort(std::execution::par)/real_time                                 113 ms          102 ms            6
pluggable_sort(poolstl::par, ..., pdqsort)/real_time               71.7 ms         6.67 ms           10
transform()/real_time                                              95.0 ms         94.9 ms            7
transform(poolstl::par)/real_time                                  17.4 ms        0.037 ms           38
transform(std::execution::par)/real_time                           15.3 ms         13.2 ms           45
exclusive_scan()/real_time                                         33.7 ms         33.7 ms           21
exclusive_scan(poolstl::par)/real_time                             11.6 ms        0.095 ms           55
exclusive_scan(std::execution::par)/real_time                      19.8 ms         15.3 ms           32
reduce()/real_time                                                 15.2 ms         15.2 ms           46
reduce(poolstl::par)/real_time                                     4.06 ms        0.044 ms          169
reduce(std::execution::par)/real_time                              3.38 ms         3.16 ms          214
```

# poolSTL as `std::execution::par`
**USE AT YOUR OWN RISK! THIS IS A HACK!**

Two-line hack for missing compiler support. A no-op on compilers with support.

If `POOLSTL_STD_SUPPLEMENT` is defined then poolSTL will check for native compiler support.
If not found then poolSTL will alias its `poolstl::par` as `std::execution::par`:

```c++
#define POOLSTL_STD_SUPPLEMENT
#include <poolstl/poolstl.hpp>
```

Now just use `std::execution::par` as normal, and poolSTL will fill in as necessary. See [supplement_test.cpp](tests/supplement_test.cpp).

**Example use case:** You *can* link against TBB, so you'll use native support on GCC 9+, Clang, MSVC, etc.
PoolSTL will fill in automatically on GCC <9 and Apple Clang.

**Example use case 2:** You'd *prefer* to use the TBB version, but don't want to fail on systems that don't have it.
Simply use the supplement as above, but have your build system (CMake, meson, etc.) check for TBB.
If not found, define `POOLSTL_STD_SUPPLEMENT_NO_INCLUDE` and the supplement will not `#include <execution>` (and neither should your code!),
thus dropping the TBB link requirement. The poolSTL supplement fills in.  
See the supplement section of [tests/CMakeLists.txt](tests/CMakeLists.txt) for an example.
