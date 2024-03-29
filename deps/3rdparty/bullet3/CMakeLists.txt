set(LIBBULLET3_BUILD_SHARED ${BUILD_SHARED_LIBS} CACHE BOOL "Build shared library" FORCE)
set(bullet_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/bullet3)

cmake_minimum_required(VERSION 3.15)

unset(PROJECT_IS_TOPLEVEL)

project(libbullet3
    VERSION 3.1.7
    DESCRIPTION "C++ library portion of bullet"
    LANGUAGES CXX
)

# PROJECT_IS_TOPLEVEL for older CMake versions
if(NOT DEFINED PROJECT_IS_TOPLEVEL)
    set(PROJECT_IS_TOPLEVEL OFF)
    if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
        set(PROJECT_IS_TOPLEVEL ON)
    endif()
endif()

# Build options
option(LIBBULLET3_BUILD_TESTS "Build libbullet3 tests" ${PROJECT_IS_TOPLEVEL})
option(LIBBULLET3_INSTALL "Install library" ${PROJECT_IS_TOPLEVEL})
option(LIBBULLET3_BUILD_SHARED "Build libbullet3 as a shared library" ${BUILD_SHARED_LIBS})

# Grab the upstream Bullet source code
# include(FetchContent)
# FetchContent_Declare(bullet
# 	GIT_REPOSITORY https://github.com/bulletphysics/bullet3.git
# 	GIT_TAG ebe1916b90acae8b13cd8c6b637d8327cdc64e94 # 3.1.7
# )
# FetchContent_Populate(bullet)

# Bullet is broken into sub-"libraries"; we'll automate handling of these
set(BULLET_SUBMODULES
    Bullet3Collision
    Bullet3Common
    Bullet3Dynamics
    Bullet3Geometry
    BulletDynamics
    BulletCollision
    BulletInverseDynamics
    BulletSoftBody
    LinearMath
)

# Find all relevant sources in upstream repo
# We don't use the bc*All.cpp files, as they
# fail to build, while the individual sources
# build correctly
set(BULLET3_SOURCES
    "${bullet_SOURCE_DIR}/src/btBulletCollisionCommon.h"
    "${bullet_SOURCE_DIR}/src/btBulletDynamicsCommon.h"
)
foreach(SUB ${BULLET_SUBMODULES})
    file(GLOB_RECURSE SUBMODULE_SOURCES
        "${bullet_SOURCE_DIR}/src/${SUB}/*.h"
        "${bullet_SOURCE_DIR}/src/${SUB}/*.cpp"
    )
    list(APPEND BULLET3_SOURCES ${SUBMODULE_SOURCES})
endforeach()

# CPack support
set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
include(CPack)

# Build the main libbullet3 library
set(BULLET3_LIBRARY_TYPE "STATIC")
if(LIBBULLET3_BUILD_SHARED)
    set(BULLET3_LIBRARY_TYPE "SHARED")
endif()
add_library(libbullet3 ${BULLET3_LIBRARY_TYPE} ${BULLET3_SOURCES})
set_target_properties(libbullet3 PROPERTIES
    WINDOWS_EXPORT_ALL_SYMBOLS 1
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION}
)
target_include_directories(libbullet3
    SYSTEM INTERFACE
        $<BUILD_INTERFACE:${bullet_SOURCE_DIR}/src>
        $<INSTALL_INTERFACE:include/Bullet3>
    PRIVATE
        ${bullet_SOURCE_DIR}/src
)

# Silence the many warnings in the libbullet source code;
# we're not the maintainers of the code, so these warnings
# aren't meaningful to us; they just clog up build output
# when building Bullet as a subdirectory
target_compile_options(libbullet3 PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/w>
    $<$<CXX_COMPILER_ID:GNU,Clang,Intel>:-w>
)


add_library(Bullet3::Bullet3 ALIAS libbullet3) # Use our library

if(LIBBULLET3_INSTALL)

# Installation
install(
    TARGETS libbullet3
    DESTINATION lib
    EXPORT Bullet3Targets
)

if(LIBBULLET3_BUILD_SHARED AND MSVC)
    install(FILES $<TARGET_PDB_FILE:libbullet3> DESTINATION lib)
endif()

install(FILES
    "${bullet_SOURCE_DIR}/src/btBulletCollisionCommon.h"
    "${bullet_SOURCE_DIR}/src/btBulletDynamicsCommon.h"
    DESTINATION "include/Bullet3"
)
foreach(SUB ${BULLET_SUBMODULES})
    install(
        DIRECTORY "${bullet_SOURCE_DIR}/src/${SUB}"
        DESTINATION "include/Bullet3"
        FILES_MATCHING PATTERN "*.h"
    )
endforeach()

install(
    EXPORT Bullet3Targets
    FILE Bullet3Targets.cmake
    DESTINATION "lib/cmake/Bullet3"
)

include(CMakePackageConfigHelpers)
configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/Config.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/Bullet3Config.cmake"
    INSTALL_DESTINATION "lib/cmake/Bullet3"
    NO_SET_AND_CHECK_MACRO
    NO_CHECK_REQUIRED_COMPONENTS_MACRO
)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/Bullet3ConfigVersion.cmake"
    VERSION "${libbullet3_VERSION_MAJOR}.${libbullet3_VERSION_MINOR}"
    COMPATIBILITY AnyNewerVersion
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/Bullet3Config.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/Bullet3ConfigVersion.cmake"
    DESTINATION lib/cmake/Bullet3
)

export(EXPORT Bullet3Targets
  FILE "${CMAKE_CURRENT_BINARY_DIR}/Bullet3Targets.cmake"
)
endif()

# Tests
if(LIBBULLET3_BUILD_TESTS)
    enable_testing()

    add_executable(test_libbullet3
        "test_bullet3.cpp"
    )
    target_link_libraries(test_libbullet3 libbullet3)
    add_test(NAME test_libbullet3 COMMAND test_libbullet3)
endif()
