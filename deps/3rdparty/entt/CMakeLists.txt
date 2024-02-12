set(ENTT_INCLUDE_HEADERS ON CACHE BOOL "Add all EnTT headers to the EnTT target." FORCE)
add_subdirectory(entt)

set (target_for_ide "entt_ide")
if (NOT TARGET ${target_for_ide})
    file(GLOB_RECURSE SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/entt/src/*.h" "${CMAKE_CURRENT_SOURCE_DIR}/entt/src/*.hpp")
    add_custom_target(${target_for_ide} SOURCES ${SOURCES})
endif()

set_target_properties(${target_for_ide} PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
    POSITION_INDEPENDENT_CODE ON
    WINDOWS_EXPORT_ALL_SYMBOLS ON
)


