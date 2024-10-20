set(target_name imgui)

file(
    GLOB #
    DEAR_IMGUI_SOURCES #
    ${CMAKE_CURRENT_SOURCE_DIR}/*.h #
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui/*.h #
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui/*.cpp #
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui/*.inl #

)

file(
    GLOB_RECURSE #
    DEAR_IMGUI_SOURCES_USER #
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui_widgets/*.h #
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui_widgets/*.cpp #
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui_widgets/*.inl #
)
set(DEAR_IMGUI_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

list(APPEND DEAR_IMGUI_SOURCES ${DEAR_IMGUI_SOURCES_USER})

add_library(${target_name} ${DEAR_IMGUI_SOURCES})

target_include_directories(${target_name}
    PUBLIC
		${DEAR_IMGUI_INCLUDE_DIR}/
)
set_target_properties(${target_name} PROPERTIES
    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
    CXX_LING_LANGUAGE C
    POSITION_INDEPENDENT_CODE ON
    WINDOWS_EXPORT_ALL_SYMBOLS ON
)
target_compile_definitions(${target_name} PUBLIC IMGUI_DEFINE_MATH_OPERATORS)
target_compile_definitions(${target_name} PUBLIC IMGUI_USE_WCHAR32)
target_compile_definitions(${target_name} PUBLIC IMGUI_DISABLE_OBSOLETE_FUNCTIONS)

if(MSVC)
	target_compile_definitions(${target_name} PRIVATE _SCL_SECURE_NO_WARNINGS)
	target_compile_definitions(${target_name} PRIVATE _CRT_SECURE_NO_WARNINGS)
else()
    set_target_properties(${target_name} PROPERTIES COMPILE_FLAGS "-Wno-unused-function -Wno-unused-variable")
    if ((CMAKE_CXX_COMPILER_ID STREQUAL "GNU") OR (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 13.0))
        set_target_properties(${target_name} PROPERTIES COMPILE_FLAGS "-Wno-unused-but-set-variable")
    endif()
endif()
