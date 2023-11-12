if (NOT DEFINED GIT_VERSION_MAJOR)
    message(ERROR " GIT_VERSION_MAJOR is not defined.")
    set(GIT_VERSION_MAJOR "1")
endif()

if (NOT DEFINED GIT_VERSION_MINOR)
    message(ERROR " GIT_VERSION_MINOR is not defined.")
    set(GIT_VERSION_MAJOR "0")
endif()

if (NOT DEFINED GIT_VERSION_PATCH)
    message(ERROR " GIT_VERSION_PATCH is not defined.")
endif()

if (NOT DEFINED GIT_VERSION)
    message(ERROR " GIT_VERSION is not defined.")
endif()


file(GLOB_RECURSE SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.c"
                          "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
                          "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
                          "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp"
                          "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp"
                          "${CMAKE_CURRENT_SOURCE_DIR}/../rc/*.rc")

add_library(version ${SOURCES})

target_compile_definitions(version PRIVATE -DVERSION_MAJOR="${GIT_VERSION_MAJOR}")
target_compile_definitions(version PRIVATE -DVERSION_MINOR="${GIT_VERSION_MINOR}")
target_compile_definitions(version PRIVATE -DVERSION_PATCH="${GIT_VERSION_PATCH}")
target_compile_definitions(version PRIVATE -DVERSION="${GIT_VERSION}")

target_include_directories(version PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
