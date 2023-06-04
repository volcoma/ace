set(BUILD_OSPP_TESTS OFF CACHE BOOL "Build the tests" FORCE)
set(BACKEND_TYPE "GLFW" CACHE STRING "Backend type chosen by the user at CMake configure time" FORCE)

add_subdirectory( ospp )
