set(BGFX_BUILD_EXAMPLES OFF CACHE BOOL "Build bgfx examples." FORCE)
set(BGFX_BUILD_TESTS OFF CACHE BOOL "Build bgfx tests." FORCE)
set(BGFX_BUILD_TESTS OFF CACHE BOOL "Create installation target." FORCE)

set(BGFX_LIBRARY_TYPE "SHARED" CACHE STRING "Linking type for library" FORCE)

if(MSVC)
    add_compile_options(/wd4244 /wd4267 /wd4291)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
endif()

if(MINGW)
    add_compile_definitions(ASTCENC_SSE=0)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
endif()

add_subdirectory(bgfx)

set_target_properties(bgfx PROPERTIES
    POSITION_INDEPENDENT_CODE ON
    WINDOWS_EXPORT_ALL_SYMBOLS ON
)

set_target_properties(bx PROPERTIES
    POSITION_INDEPENDENT_CODE ON
    WINDOWS_EXPORT_ALL_SYMBOLS ON
)

set_target_properties(bimg PROPERTIES
    POSITION_INDEPENDENT_CODE ON
    WINDOWS_EXPORT_ALL_SYMBOLS ON
)
