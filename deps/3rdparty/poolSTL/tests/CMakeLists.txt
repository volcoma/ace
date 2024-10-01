# FetchContent requires cmake >=3.11
cmake_minimum_required(VERSION 3.11...3.26)
enable_testing()

# Code Coverage
option(POOLSTL_TEST_COVERAGE "Code-coverage" OFF)
if(POOLSTL_TEST_COVERAGE)
    if (EMSCRIPTEN)
        # Clang's code coverage tools not supported on Emscripten (will produce link error).
    elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
        add_compile_options("-O0" "--coverage")
        add_link_options("-lgcov" "--coverage")
    elseif ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
        add_compile_options("-O0" "--coverage")
        add_link_options("--coverage")
    endif()
endif()

if (WIN32)
    # Need bigobj because some tests use lots of templates
    if (MSVC)
        add_compile_options("/bigobj")
    elseif(MINGW)
        # Also add -Os to reduce maximum filesize
        add_compile_options("-Wa,-mbig-obj" "-Os")
    endif()
else()
    # Be pedantic for clean code
    if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
        add_compile_options("-g" "-Wall" "-Wextra" "-pedantic")
    endif()
endif()

if (MINGW AND WIN32)
    # The MinGW on GitHub Actions requires these.
    # If omitted some executables crash with 'Exit code 0xc0000139' which appears to mean a missing DLL.
    add_link_options("-static-libstdc++" "-static-libgcc")
endif()

# install Catch2
Include(FetchContent)

FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG        v3.4.0
        GIT_SHALLOW TRUE
        EXCLUDE_FROM_ALL
)

FetchContent_MakeAvailable(Catch2)

list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
include(CTest)
include(Catch)


if (EMSCRIPTEN)
    # Includes a test that throws exceptions, which are opt-in on Emscripten.
    # See https://emscripten.org/docs/porting/exceptions.html
    add_compile_options("-fexceptions")
    add_link_options("-fexceptions")
endif()

# tests compiled with C++14 (Catch2 requires C++14)
add_executable(poolstl_test poolstl_test.cpp)
target_link_libraries(poolstl_test PRIVATE Catch2::Catch2WithMain poolSTL::poolSTL)
target_compile_definitions(poolstl_test PRIVATE CATCH_CONFIG_FAST_COMPILE)
target_compile_features(poolstl_test PUBLIC cxx_std_14)
catch_discover_tests(poolstl_test)

# tests compiled with C++17 (Some supported methods have been added to the standard library)
add_executable(poolstl_test_cpp17 poolstl_test.cpp)
target_link_libraries(poolstl_test_cpp17 PRIVATE Catch2::Catch2WithMain poolSTL::poolSTL)
target_compile_definitions(poolstl_test_cpp17 PRIVATE CATCH_CONFIG_FAST_COMPILE)
target_compile_features(poolstl_test_cpp17 PUBLIC cxx_std_17)
catch_discover_tests(poolstl_test_cpp17)

# Dedicated target to ensure C++11 builds work
add_executable(cpp11_test cpp11_test.cpp)
target_link_libraries(cpp11_test PUBLIC poolSTL::poolSTL)
target_compile_features(cpp11_test PUBLIC cxx_std_11)

# Test seq_fwd policy
add_executable(seq_fwd_test seq_fwd_test.cpp)
target_link_libraries(seq_fwd_test PRIVATE Catch2::Catch2WithMain poolSTL::poolSTL)
target_compile_definitions(seq_fwd_test PRIVATE CATCH_CONFIG_FAST_COMPILE)
target_compile_features(seq_fwd_test PUBLIC cxx_std_17)
catch_discover_tests(seq_fwd_test)


# Test std::execution supplementation.
# The test code uses only std::execution::par, seq, and par_unseq.
# On compilers with support poolSTL does nothing, else poolSTL aliases poolstl::par to the std::execution policies.
# This way users may write code using standard std::execution and still support platforms lacking support.
add_executable(supplement_test supplement_test.cpp)
target_link_libraries(supplement_test PUBLIC poolSTL::poolSTL)
target_compile_features(supplement_test PUBLIC cxx_std_17)
if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    # GCC and Clang require TBB for std::execution::par.
    # MinGW will compile std::execution::par without TBB, but performance is sequential. (Verified as of MinGW 13).
    find_package(TBB)
    if (TBB_FOUND)
        target_link_libraries(supplement_test PUBLIC TBB::tbb)
    else()
        message("No TBB")
        # Prevent including <execution>, as doing that requires linking against TBB on newer GCC/Clang.
        # User code must not #include <execution> either.
        target_compile_definitions(supplement_test PUBLIC POOLSTL_STD_SUPPLEMENT_NO_INCLUDE)
        if (MINGW)
            # extra hack!
            # MinGW declares support, so override poolSTL's auto-detection to enable the supplement.
            target_compile_definitions(supplement_test PUBLIC POOLSTL_STD_SUPPLEMENT_FORCE)
        endif()
    endif()
endif()
