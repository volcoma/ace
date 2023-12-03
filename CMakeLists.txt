cmake_minimum_required(VERSION 3.16)

project (ace)
include(CTest)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/utility")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/git")


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


include(GetGitRevisionDescription)
git_describe(GIT_VERSION --tags)

#parse the version information into pieces.
string(REGEX REPLACE "^v([0-9]+)\\..*" "\\1" GIT_VERSION_MAJOR "${GIT_VERSION}")
string(REGEX REPLACE "^v[0-9]+\\.([0-9]+).*" "\\1" GIT_VERSION_MINOR "${GIT_VERSION}")
string(REGEX REPLACE "^v[0-9]+\\.[0-9]+\\.([0-9]+).*" "\\1" GIT_VERSION_PATCH "${GIT_VERSION}")
string(REGEX REPLACE "^v[0-9]+\\.[0-9]+\\.[0-9]+(.*)" "\\1" GIT_VERSION_SHA1 "${GIT_VERSION}")
set(VERSION_SHORT "${GIT_VERSION_MAJOR}.${GIT_VERSION_MINOR}.${GIT_VERSION_PATCH}")

message(STATUS "Version ${GIT_VERSION}")

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


set(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_SKIP TRUE)
include(InstallRequiredSystemLibraries)

if(MINGW)
    get_filename_component( Mingw_Path ${CMAKE_CXX_COMPILER} PATH )
    file(GLOB MINGW_SYSTEM_RUNTIME_LIBS "${Mingw_Path}/*.dll")
    set(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS} ${MINGW_SYSTEM_RUNTIME_LIBS})
endif()
message(STATUS ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS})

if(WIN32)
    if(NOT CMAKE_BUILD_TYPE MATCHES Debug)
        if(MSVC)
            set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
        endif()

        if(MINGW)
            set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -mwindows")
        endif()
    endif()
elseif(UNIX)
    set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -rdynamic -lrt -Wl,-rpath,'$ORIGIN'" )
    set (CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -rdynamic -lrt -Wl,-rpath,'$ORIGIN'" )
endif()


#include(cmake/modules.cmake)

include(cmake/utility/target_ide_support.cmake)

add_subdirectory(deps)
#add_subdirectory(version)
add_subdirectory(engine)
add_subdirectory(editor)
add_subdirectory(launcher)

add_custom_target(copy_resources ALL
    COMMAND ${CMAKE_COMMAND} -E copy ${BGFX_DIR}/src/bgfx_shader.sh ${PROJECT_SOURCE_DIR}/engine_data/data/shaders
    COMMAND ${CMAKE_COMMAND} -E copy ${BGFX_DIR}/src/bgfx_compute.sh ${PROJECT_SOURCE_DIR}/engine_data/data/shaders

    COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/engine_data ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/data/engine
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${PROJECT_SOURCE_DIR}/editor_data ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/data/editor
    )

if(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS)
    install( PROGRAMS ${CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS} DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY} )
endif()

install(CODE "execute_process(
    COMMAND ${CMAKE_COMMAND} -E tar cfv ${PROJECT_BINARY_DIR}/${GIT_VERSION}.zip --format=zip -- .
    WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
    )")

install(CODE "MESSAGE(\"-- Package ${GIT_VERSION}.zip - done\")")
