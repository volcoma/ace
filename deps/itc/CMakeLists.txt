set(BUILD_ITC_TESTS OFF CACHE BOOL "Build the tests" FORCE)
set(BUILD_ITC_SHARED ${BUILD_SHARED_LIBS} CACHE BOOL "Build shared library" FORCE)

add_subdirectory( itc )
