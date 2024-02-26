set(YAML_BUILD_SHARED_LIBS ${BUILD_SHARED_LIBS} CACHE BOOL "Build yaml-cpp shared library" FORCE)

if(MSVC)
    add_compile_options(/wd4251 /wd4275 /wd4996 /wd4701 /wd4305 /wd4267 /wd4127 /wd4456 /wd4458)
endif()

add_subdirectory(yaml-cpp)

