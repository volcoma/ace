file(GLOB_RECURSE libsrc *.h *.cpp *.hpp *.c *.cc)

set(target_name graphics)

add_library (${target_name} ${libsrc})

target_include_directories(${target_name}
    PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_link_libraries(${target_name} PUBLIC bgfx bimg bimg_decode bimg_encode base hpp)

set_target_properties(${target_name} PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
    POSITION_INDEPENDENT_CODE ON
    WINDOWS_EXPORT_ALL_SYMBOLS ON
)

#target_precompile_headers(${target_name}
#    PUBLIC
#    <vector>
#    <memory>
#    <cstdint>
#    <string>
#    <unordered_map>
#    <limits>
#)
