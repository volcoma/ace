set(BUILD_AUDIOPP_TESTS OFF CACHE BOOL "Build the tests" FORCE)
set(BUILD_AUDIOPP_SHARED ${BUILD_SHARED_LIBS} CACHE BOOL "Build shared library" FORCE)

add_subdirectory( audiopp )
