file(GLOB_RECURSE libsrc *.h *.cpp *.hpp *.inl *.c *.cc *.m)

add_library (filedialog ${libsrc})

# Add definitions
set_target_properties(filedialog PROPERTIES
    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
    POSITION_INDEPENDENT_CODE ON
    WINDOWS_EXPORT_ALL_SYMBOLS ON
)

if(MSVC)
	target_compile_definitions(filedialog PUBLIC _CRT_SECURE_NO_WARNINGS _CRT_SECURE_NO_DEPRECATE)
endif()

target_include_directories(filedialog PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/..)
