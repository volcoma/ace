cmake_minimum_required(VERSION 3.12...3.26)

# Read the version from header
set(VERSION_HEADER_FILE include/poolstl/internal/utils.hpp)
file(STRINGS ${VERSION_HEADER_FILE} VERSION_MAJOR_STR REGEX "define .*_VERSION_MAJOR")
file(STRINGS ${VERSION_HEADER_FILE} VERSION_MINOR_STR REGEX "define .*_VERSION_MINOR")
file(STRINGS ${VERSION_HEADER_FILE} VERSION_PATCH_STR REGEX "define .*_VERSION_PATCH")
string(REGEX MATCH "[0-9]+" VERSION_MAJOR ${VERSION_MAJOR_STR})
string(REGEX MATCH "[0-9]+" VERSION_MINOR ${VERSION_MINOR_STR})
string(REGEX MATCH "[0-9]+" VERSION_PATCH ${VERSION_PATCH_STR})

project(poolSTL VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}" LANGUAGES CXX)

# Main
add_library(poolSTL INTERFACE)
# add alias so the project can be used with add_subdirectory
add_library(poolSTL::poolSTL ALIAS poolSTL)

include(GNUInstallDirs)

target_include_directories(
        ${PROJECT_NAME}
        INTERFACE
        $<BUILD_INTERFACE:${${PROJECT_NAME}_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>)

set(HEADER_FILES
        include/poolstl/poolstl.hpp
        include/poolstl/algorithm
        include/poolstl/execution
        include/poolstl/numeric
        include/poolstl/variant_policy.hpp
        include/poolstl/iota_iter.hpp
        include/poolstl/internal/utils.hpp
        include/poolstl/internal/ttp_impl.hpp
        include/poolstl/internal/thread_impl.hpp
        include/poolstl/internal/task_thread_pool.hpp)

set_target_properties(${PROJECT_NAME} PROPERTIES PUBLIC_HEADER "${HEADER_FILES}")

target_compile_features(poolSTL INTERFACE cxx_std_11)

# Some older compilers (eg. Clang, GCC <9) require extra link flags for C++11 threads
find_package(Threads)
target_link_libraries(poolSTL INTERFACE Threads::Threads)

if (EMSCRIPTEN)
    # Emscripten needs -pthread compiler and linker switch for C++11 threads.
    # Anything that will link against poolSTL also needs to be compiled with -pthread.
    add_compile_options("-pthread")
    target_compile_options(poolSTL INTERFACE "-pthread")
    target_link_options(poolSTL INTERFACE "-pthread")
endif()

###############################################

# Tests
option(POOLSTL_TEST "Enable poolSTL tests" OFF)
if(POOLSTL_TEST)
    add_subdirectory(tests)
endif()

# Benchmarks
option(POOLSTL_BENCH "Enable poolSTL benchmarks" OFF)
if(POOLSTL_BENCH)
    add_subdirectory(benchmark)
endif()

###############################################
# For cmake install:

include(CMakePackageConfigHelpers)

# Create version file
write_basic_package_version_file("${PROJECT_NAME}ConfigVersion.cmake"
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion)

# Create a configuration file
configure_package_config_file(
        "${PROJECT_SOURCE_DIR}/cmake/config.cmake.in"
        "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
        INSTALL_DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/cmake/${PROJECT_NAME}
        PATH_VARS CMAKE_INSTALL_INCLUDEDIR)

# Install config and version files
install(FILES "${PROJECT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
        "${PROJECT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
        DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/cmake/${PROJECT_NAME})

# Define the install targets
install(TARGETS ${PROJECT_NAME}
        EXPORT ${PROJECT_NAME}Targets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME})

# Install the targets
install(EXPORT ${PROJECT_NAME}Targets
        FILE ${PROJECT_NAME}Targets.cmake
        NAMESPACE ${PROJECT_NAME}::
        DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/cmake/${PROJECT_NAME})
