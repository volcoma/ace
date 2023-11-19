set(target_name mesh_import)

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

target_link_libraries(${target_name} PUBLIC assimp)

target_include_directories(${target_name} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
