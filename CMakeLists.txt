cmake_minimum_required(VERSION 3.26)

project (ace)
include(CTest)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/utility")

if(NOT CMAKE_RUNTIME_OUTPUT_DIRECTORY)
	set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
endif()
if(NOT CMAKE_LIBRARY_OUTPUT_DIRECTORY)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
endif()
if(NOT CMAKE_ARCHIVE_OUTPUT_DIRECTORY)
	set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
endif()

message(STATUS "CMAKE_RUNTIME_OUTPUT_DIRECTORY=${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
message(STATUS "CMAKE_ARCHIVE_OUTPUT_DIRECTORY=${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}")
message(STATUS "CMAKE_LIBRARY_OUTPUT_DIRECTORY=${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")

option(BUILD_ENGINE_SHARED "Build as a shared library." ON)
option(BUILD_ENGINE_TESTS "Build the tests" OFF)
option(BUILD_ENGINE_WITH_CODE_STYLE_CHECKS "Build with code style checks." OFF)

if(BUILD_ENGINE_SHARED)
	set(BUILD_SHARED_LIBS ON)
endif()

if(BUILD_ENGINE_WITH_CODE_STYLE_CHECKS)
	set(BUILD_WITH_CODE_STYLE_CHECKS ON)
endif()


set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
#set(CMAKE_SUPPORT_WINDOWS_EXPORT_ALL_SYMBOLS 1)

if(MSVC)
    add_compile_options("/Zc:__cplusplus")
endif()

if(MINGW)
    add_compile_options("-Wl,--export-all-symbols")
endif()


#include(cmake/modules.cmake)

include(cmake/utility/target_ide_support.cmake)

add_subdirectory(deps)
add_subdirectory(engine)
add_subdirectory(editor)
add_subdirectory(launcher)
