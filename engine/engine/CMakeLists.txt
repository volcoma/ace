file(GLOB_RECURSE libsrc *.h *.cpp *.hpp *.c *.cc)

set(target_name engine)

add_library(${target_name} ${libsrc})

target_include_directories(${target_name}
    PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_link_libraries (${target_name}
    PUBLIC
    EnTT::EnTT
    Edyn::Edyn
    #ndNewton
    Bullet3::Bullet3
    service
    context
    logging
    cmd_line
    memory
    ospp
    graphics
    simulation
    serialization
    reflection
    filesystem
    math
    uuidpp
    itc
    assimp
    string_utils)

target_precompile_headers(${target_name}
    PUBLIC
    pch.h
)

set_target_properties(${target_name} PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
    POSITION_INDEPENDENT_CODE ON
    WINDOWS_EXPORT_ALL_SYMBOLS ON
)
