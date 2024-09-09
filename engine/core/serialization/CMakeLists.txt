file(GLOB_RECURSE libsrc *.h *.cpp *.hpp *.c *.cc)

set(target_name serialization)

add_library(${target_name} ${libsrc})
target_link_libraries(${target_name} PUBLIC cereal::cereal ser20::ser20 yaml-cpp hpp)

target_include_directories(${target_name}
    PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}/..
)

set_target_properties(${target_name} PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
    POSITION_INDEPENDENT_CODE ON
    WINDOWS_EXPORT_ALL_SYMBOLS ON
)
