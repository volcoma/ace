#pragma once

#include "config.hpp"

////////////////////////////////////////////////////////////
// Define helpers to create portable import / export macros for each module
////////////////////////////////////////////////////////////
#if !defined(ACE_STATIC)

#if ACE_ON(ACE_PLATFORM_WINDOWS)
// Windows compilers need specific (and different) keywords for export and import
#define ACE_API_EXPORT __declspec(dllexport)
#define ACE_API_IMPORT __declspec(dllimport)

// For Visual C++ compilers, we also need to turn off this annoying C4251 warning
#if ACE_ON(ACE_COMPILER_MSVC)
#pragma warning(disable : 4251)
#endif

#else // Linux, FreeBSD, Mac OS X

// GCC 4+ has special keywords for showing/hidding symbols,
// the same keyword is used for both importing and exporting
#define ACE_API_EXPORT __attribute__((__visibility__("default")))
#define ACE_API_IMPORT __attribute__((__visibility__("default")))

#endif

#else

// Static build doesn't need import/export macros
#define ACE_API_EXPORT
#define ACE_API_IMPORT

#endif

////////////////////////////////////////////////////////////
// Define portable import / export macros
////////////////////////////////////////////////////////////
#if defined(ACE_API_EXPORTS)
#define ACE_API ACE_API_EXPORT
#else
#define ACE_API ACE_API_IMPORT
#endif
