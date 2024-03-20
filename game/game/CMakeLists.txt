file(GLOB_RECURSE libsrc *.h *.cpp *.hpp *.c *.cc)

set(target_name game)



add_executable(${target_name} ${libsrc})

target_include_directories(${target_name}
    PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}/..
)
add_dependencies(${target_name} engine)
target_link_libraries(${target_name} PUBLIC engine)
#target_precompile_headers(${target_name} REUSE_FROM engine)

set_target_properties(${target_name} PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
    POSITION_INDEPENDENT_CODE ON
    WINDOWS_EXPORT_ALL_SYMBOLS ON
)

#if (CMAKE_COMPILER_IS_GNUCXX)
#    # we have to use this flag, otherwise dlclose does not call the destructor function
#    # of the library
#    target_compile_options(${target_name} PRIVATE "-fno-gnu-unique")
#endif()


if(WIN32)
    if(MSVC)
        target_link_options(${target_name} PRIVATE "/SUBSYSTEM:WINDOWS")
        target_link_options(${target_name} PRIVATE "/ENTRY:mainCRTStartup")
    endif()

    if(MINGW)
        target_link_options(${target_name} PRIVATE "-mwindows")
    endif()
elseif(UNIX)

endif()
