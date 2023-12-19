cmake_minimum_required(VERSION 3.14)

set(target_name filesystem)

file(GLOB_RECURSE SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.c"
                          "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
                          "${CMAKE_CURRENT_SOURCE_DIR}/*.h"
                          "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp")
add_library(${target_name} ${SOURCES})

# Add definitions
set_target_properties(${target_name} PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
    POSITION_INDEPENDENT_CODE ON
    WINDOWS_EXPORT_ALL_SYMBOLS ON
)

set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)
target_link_libraries(${target_name} PUBLIC Threads::Threads)

target_link_libraries(${target_name} PUBLIC hpp)

target_include_directories(${target_name} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
