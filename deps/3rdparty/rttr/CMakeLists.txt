if(BUILD_SHARED_LIBS)
    set(BUILD_RTTR_DYNAMIC ON CACHE BOOL "Build the dynamic/shared version of RTTR library" FORCE)
    set(BUILD_STATIC OFF CACHE BOOL "Build RTTR as static library" FORCE)
else()
    set(BUILD_RTTR_DYNAMIC OFF CACHE BOOL "Build the dynamic/shared version of RTTR library" FORCE)
    set(BUILD_STATIC ON CACHE BOOL "Build RTTR as static library" FORCE)
endif()

set(BUILD_UNIT_TESTS OFF CACHE BOOL "Build the unit tests of RTTR" FORCE)
set(BUILD_WITH_STATIC_RUNTIME_LIBS OFF CACHE BOOL "Link against the static runtime libraries" FORCE)
set(BUILD_WITH_RTTI ON CACHE BOOL "Enable build with C++ runtime type information for compilation" FORCE)
set(BUILD_BENCHMARKS OFF CACHE BOOL "Enable this to build the benchmarks" FORCE)
set(BUILD_EXAMPLES OFF CACHE BOOL "Enable this to build the examples" FORCE)
set(BUILD_DOCUMENTATION OFF CACHE BOOL "Enable this to build the documentation" FORCE)
set(BUILD_INSTALLER OFF CACHE BOOL "Enable this to build the installer" FORCE)
set(BUILD_PACKAGE OFF CACHE BOOL "Enable this to build the installer" FORCE)
set(USE_PCH OFF CACHE BOOL "Use precompiled header files for compilation" FORCE)
set(CUSTOM_DOXYGEN_STYLE OFF CACHE BOOL "Enable this option to use a custom doxygen style for HTML documentation; Otherwise the default will be used" FORCE)
set(BUILD_WEBSITE_DOCU OFF CACHE BOOL "Enable this option to create the special docu for the website" FORCE)
set(CMAKE_REQUIRED_DEFINITIONS ${CMAKE_CXX14_STANDARD_COMPILE_OPTION})




if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    add_compile_options(-Wno-implicit-int-float-conversion)
  # using Clang
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    add_compile_options(-Wno-attributes)
  # using GCC
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
  # using Intel C++
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  # using Visual Studio C++
endif()

add_subdirectory( rttr )


if(BUILD_SHARED_LIBS)
    add_library(RTTR::RTTR ALIAS rttr_core)
else()
    add_library(RTTR::RTTR ALIAS rttr_core_lib)
endif()





#target_precompile_headers(rttr_core
#    PRIVATE
#    rttr/src/rttr/detail/base/pch.h
#)

