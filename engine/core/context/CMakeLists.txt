file(GLOB_RECURSE libsrc *.h *.cpp *.hpp *.c *.cc)

set(target_name context)

add_library(${target_name} INTERFACE)

target_link_libraries(${target_name} INTERFACE hpp type_index)

target_include_directories(${target_name}
    INTERFACE
		${CMAKE_CURRENT_SOURCE_DIR}/..
)

set (target_for_ide "${target_name}_ide")
if (NOT TARGET ${target_for_ide})
    add_custom_target(${target_for_ide} SOURCES ${libsrc})
endif()

#set_target_properties(${target_name} PROPERTIES
#    CXX_STANDARD 20
#    CXX_STANDARD_REQUIRED YES
#    CXX_EXTENSIONS NO
#    POSITION_INDEPENDENT_CODE ON
#    WINDOWS_EXPORT_ALL_SYMBOLS ON
#)
