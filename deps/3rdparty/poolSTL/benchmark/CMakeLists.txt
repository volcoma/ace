# FetchContent requires cmake >=3.11
cmake_minimum_required(VERSION 3.11...3.26)

if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS_DEBUG "-g -Wall -Wextra -pedantic")
    set(CMAKE_CXX_FLAGS_RELEASE "-g -O3 -DNDEBUG")
endif ()

if (MINGW AND WIN32)
    # MinGW on GitHub Actions requires these.
    # If omitted the executable crashes with 'Exit code 0xc0000139' which appears to mean a missing DLL.
    add_link_options("-static-libstdc++" "-static-libgcc")
endif()

include(FetchContent)

# Add Google Benchmark
set(BENCHMARK_ENABLE_TESTING NO)

include(FetchContent)

FetchContent_Declare(
        googlebenchmark
        GIT_REPOSITORY https://github.com/google/benchmark.git
        GIT_TAG origin/main
        GIT_SHALLOW TRUE
        EXCLUDE_FROM_ALL
)

FetchContent_MakeAvailable(googlebenchmark)

# benchmark
add_executable(poolstl_bench
        main.cpp
        utils.hpp
        algorithm_bench.cpp
        numeric_bench.cpp
        )
target_link_libraries(poolstl_bench benchmark::benchmark poolSTL::poolSTL)
target_compile_features(poolstl_bench PUBLIC cxx_std_17)

if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang" OR ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" AND NOT MINGW))
    # GCC and Clang require TBB for native std::execution::par
    find_package(TBB)
    if (TBB_FOUND)
        message("Found TBB")
        target_link_libraries(poolstl_bench TBB::tbb)
        target_compile_definitions(poolstl_bench PUBLIC POOLSTL_BENCH_STD_PAR)
    else()
        message("No TBB")
    endif()
else()
    # MSVC requires nothing.
    # MinGW also requires nothing to compile std::execution::par, but performance appears to be the same as sequential.
    target_compile_definitions(poolstl_bench PUBLIC POOLSTL_BENCH_STD_PAR)
endif()

if (EMSCRIPTEN)
    # Benchmark uses large arrays that require more than Emscripten's default 16MB.
    target_link_options(poolstl_bench PUBLIC "-sTOTAL_MEMORY=2048MB")

    # Recommended by docs, but does not appear to have an effect. Maybe new node.js does not need it?
    # target_link_options(poolstl_bench PUBLIC "-sPTHREAD_POOL_SIZE=navigator.hardwareConcurrency")
endif()
