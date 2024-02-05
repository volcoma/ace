set(BGFX_BUILD_EXAMPLES OFF CACHE BOOL "Build bgfx examples." FORCE)
set(BGFX_BUILD_TESTS OFF CACHE BOOL "Build bgfx tests." FORCE)
set(BGFX_BUILD_INSTALL OFF CACHE BOOL "Create installation target." FORCE)

set(BGFX_LIBRARY_TYPE "SHARED" CACHE STRING "Linking type for library" FORCE)
set(BGFX_BUILD_TOOLS_BIN2C ON CACHE BOOL "Build bx binary to c converter." FORCE)
set(BGFX_BUILD_TOOLS_SHADER ON CACHE BOOL "Build bgfx shader tools." FORCE)
set(BGFX_BUILD_TOOLS_GEOMETRY ON CACHE BOOL "Build bgfx geometry tools." FORCE)
set(BGFX_BUILD_TOOLS_TEXTURE ON CACHE BOOL "Build bgfx texture tools." FORCE)

if(MSVC)
    add_compile_options(/wd4244 /wd4267 /wd4291)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
endif()

if(MINGW)
    add_compile_definitions(ASTCENC_SSE=0)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)

    set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--export-all-symbols" )
    set (CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--export-all-symbols" )
endif()

add_definitions(-DBGFX_CONFIG_MAX_SHADERS=4096)
add_definitions(-DBGFX_CONFIG_MAX_UNIFORMS=4096)
add_definitions(-DBGFX_CONFIG_MAX_FRAME_BUFFERS=16384)
add_definitions(-DBGFX_CONFIG_MAX_TEXTURES=16384)
add_definitions(-DBGFX_CONFIG_MAX_VERTEX_LAYOUTS=512)
add_definitions(-DBGFX_CONFIG_SORT_KEY_NUM_BITS_PROGRAM=10)


add_subdirectory(bgfx)

set_target_properties(bgfx PROPERTIES
    POSITION_INDEPENDENT_CODE ON
    WINDOWS_EXPORT_ALL_SYMBOLS ON
    #CXX_FLAGS "-Wl,--export-all-symbols"
)

set_target_properties(bx PROPERTIES
    POSITION_INDEPENDENT_CODE ON
    WINDOWS_EXPORT_ALL_SYMBOLS ON
)

set_target_properties(bimg PROPERTIES
    POSITION_INDEPENDENT_CODE ON
    WINDOWS_EXPORT_ALL_SYMBOLS ON
)

