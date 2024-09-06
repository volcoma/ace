file(GLOB_RECURSE libsrc *.h *.cpp *.hpp *.c *.cc)

set(target_name reflection)

add_library(${target_name} ${libsrc})


include(GenerateExportHeader)
generate_export_header(${target_name})

target_link_libraries(${target_name} PUBLIC RTTR::RTTR)

target_include_directories(${target_name}
    PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_include_directories(${target_name}
    PUBLIC
        ${CMAKE_CURRENT_BINARY_DIR}/..
)

set_target_properties(${target_name} PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
    POSITION_INDEPENDENT_CODE ON
    #WINDOWS_EXPORT_ALL_SYMBOLS ON
)
