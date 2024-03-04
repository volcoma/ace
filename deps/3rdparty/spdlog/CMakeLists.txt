set(SPDLOG_MASTER_PROJECT OFF)
set(SPDLOG_BUILD_SHARED ${BUILD_SHARED_LIBS} CACHE BOOL "Build shared library" FORCE)
set(SPDLOG_BUILD_PIC ON CACHE BOOL "Build position independent code (-fPIC)" FORCE)

add_subdirectory( spdlog )

