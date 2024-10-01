#pragma once
#include <cstdint>

#if !defined(DEBUG) && !defined(_DEBUG)
#ifndef NDEBUG
#define NDEBUG
#endif
#ifndef _NDEBUG
#define _NDEBUG
#endif
#endif


// Architecture
#define ACE_ARCH_32BIT 0
#define ACE_ARCH_64BIT 0

// Compiler
#define ACE_COMPILER_CLANG          0
#define ACE_COMPILER_CLANG_ANALYZER 0
#define ACE_COMPILER_GCC            0
#define ACE_COMPILER_MSVC           0

// Endianness
#define ACE_CPU_ENDIAN_BIG    0
#define ACE_CPU_ENDIAN_LITTLE 0

// CPU
#define ACE_CPU_ARM   0
#define ACE_CPU_JIT   0
#define ACE_CPU_MIPS  0
#define ACE_CPU_PPC   0
#define ACE_CPU_RISCV 0
#define ACE_CPU_X86   0

// C Runtime
#define ACE_CRT_BIONIC 0
#define ACE_CRT_GLIBC  0
#define ACE_CRT_LIBCXX 0
#define ACE_CRT_MINGW  0
#define ACE_CRT_MSVC   0
#define ACE_CRT_NEWLIB 0

#ifndef ACE_CRT_NONE
#	define ACE_CRT_NONE 0
#endif // ACE_CRT_NONE

// Language standard version
#define ACE_LANGUAGE_CPP17 201703L
#define ACE_LANGUAGE_CPP20 202002L
#define ACE_LANGUAGE_CPP23 202207L

// Platform
#define ACE_PLATFORM_ANDROID    0
#define ACE_PLATFORM_BSD        0
#define ACE_PLATFORM_EMSCRIPTEN 0
#define ACE_PLATFORM_HAIKU      0
#define ACE_PLATFORM_HURD       0
#define ACE_PLATFORM_IOS        0
#define ACE_PLATFORM_LINUX      0
#define ACE_PLATFORM_NX         0
#define ACE_PLATFORM_OSX        0
#define ACE_PLATFORM_PS4        0
#define ACE_PLATFORM_PS5        0
#define ACE_PLATFORM_RPI        0
#define ACE_PLATFORM_VISIONOS   0
#define ACE_PLATFORM_WINDOWS    0
#define ACE_PLATFORM_WINRT      0
#define ACE_PLATFORM_XBOXONE    0

// http://sourceforge.net/apps/mediawiki/predef/index.php?title=Compilers
#if defined(__clang__)
// clang defines __GNUC__ or _MSC_VER
#	undef  ACE_COMPILER_CLANG
#	define ACE_COMPILER_CLANG (__clang_major__ * 10000 + __clang_minor__ * 100 + __clang_patchlevel__)
#	if defined(__clang_analyzer__)
#		undef  ACE_COMPILER_CLANG_ANALYZER
#		define ACE_COMPILER_CLANG_ANALYZER 1
#	endif // defined(__clang_analyzer__)
#elif defined(_MSC_VER)
#	undef  ACE_COMPILER_MSVC
#	define ACE_COMPILER_MSVC _MSC_VER
#elif defined(__GNUC__)
#	undef  ACE_COMPILER_GCC
#	define ACE_COMPILER_GCC (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
#else
#	error "ACE_COMPILER_* is not defined!"
#endif //

// http://sourceforge.net/apps/mediawiki/predef/index.php?title=Architectures
#if defined(__arm__)     \
 || defined(__aarch64__) \
 || defined(_M_ARM)
#	undef  ACE_CPU_ARM
#	define ACE_CPU_ARM 1
#	define ACE_CACHE_LINE_SIZE 64
#elif defined(__MIPSEL__)     \
 ||   defined(__mips_isa_rev) \
 ||   defined(__mips64)
#	undef  ACE_CPU_MIPS
#	define ACE_CPU_MIPS 1
#	define ACE_CACHE_LINE_SIZE 64
#elif defined(_M_PPC)        \
 ||   defined(__powerpc__)   \
 ||   defined(__powerpc64__)
#	undef  ACE_CPU_PPC
#	define ACE_CPU_PPC 1
#	define ACE_CACHE_LINE_SIZE 128
#elif defined(__riscv)   \
 ||   defined(__riscv__) \
 ||   defined(RISCVEL)
#	undef  ACE_CPU_RISCV
#	define ACE_CPU_RISCV 1
#	define ACE_CACHE_LINE_SIZE 64
#elif defined(_M_IX86)    \
 ||   defined(_M_X64)     \
 ||   defined(__i386__)   \
 ||   defined(__x86_64__)
#	undef  ACE_CPU_X86
#	define ACE_CPU_X86 1
#	define ACE_CACHE_LINE_SIZE 64
#else // PNaCl doesn't have CPU defined.
#	undef  ACE_CPU_JIT
#	define ACE_CPU_JIT 1
#	define ACE_CACHE_LINE_SIZE 64
#endif //

#if defined(__x86_64__)    \
 || defined(_M_X64)        \
 || defined(__aarch64__)   \
 || defined(__64BIT__)     \
 || defined(__mips64)      \
 || defined(__powerpc64__) \
 || defined(__ppc64__)     \
 || defined(__LP64__)
#	undef  ACE_ARCH_64BIT
#	define ACE_ARCH_64BIT 64
#else
#	undef  ACE_ARCH_32BIT
#	define ACE_ARCH_32BIT 32
#endif //

#if ACE_CPU_PPC
// __BIG_ENDIAN__ is gcc predefined macro
#	if defined(__BIG_ENDIAN__)
#		undef  ACE_CPU_ENDIAN_BIG
#		define ACE_CPU_ENDIAN_BIG 1
#	else
#		undef  ACE_CPU_ENDIAN_LITTLE
#		define ACE_CPU_ENDIAN_LITTLE 1
#	endif
#else
#	undef  ACE_CPU_ENDIAN_LITTLE
#	define ACE_CPU_ENDIAN_LITTLE 1
#endif // ACE_CPU_PPC

// http://sourceforge.net/apps/mediawiki/predef/index.php?title=Operating_Systems
#if defined(_DURANGO) || defined(_XBOX_ONE)
#	undef  ACE_PLATFORM_XBOXONE
#	define ACE_PLATFORM_XBOXONE 1
#elif defined(_WIN32) || defined(_WIN64)
// http://msdn.microsoft.com/en-us/library/6sehtctf.aspx
#	ifndef NOMINMAX
#		define NOMINMAX
#	endif // NOMINMAX
//  If _USING_V110_SDK71_ is defined it means we are using the v110_xp or v120_xp toolset.
#	if defined(_MSC_VER) && (_MSC_VER >= 1700) && !defined(_USING_V110_SDK71_)
#		include <winapifamily.h>
#	endif // defined(_MSC_VER) && (_MSC_VER >= 1700) && (!_USING_V110_SDK71_)
#	if !defined(WINAPI_FAMILY) || (WINAPI_FAMILY == WINAPI_FAMILY_DESKTOP_APP)
#		undef  ACE_PLATFORM_WINDOWS
#		if !defined(WINVER) && !defined(_WIN32_WINNT)
#			if ACE_ARCH_64BIT
//				When building 64-bit target Win7 and above.
#				define WINVER 0x0601
#				define _WIN32_WINNT 0x0601
#			else
//				Windows Server 2003 with SP1, Windows XP with SP2 and above
#				define WINVER 0x0502
#				define _WIN32_WINNT 0x0502
#			endif // ACE_ARCH_64BIT
#		endif // !defined(WINVER) && !defined(_WIN32_WINNT)
#		define ACE_PLATFORM_WINDOWS _WIN32_WINNT
#	else
#		undef  ACE_PLATFORM_WINRT
#		define ACE_PLATFORM_WINRT 1
#	endif
#elif defined(__ANDROID__)
// Android compiler defines __linux__
#	include <sys/cdefs.h> // Defines __BIONIC__ and includes android/api-level.h
#	undef  ACE_PLATFORM_ANDROID
#	define ACE_PLATFORM_ANDROID __ANDROID_API__
#elif defined(__VCCOREVER__)
// RaspberryPi compiler defines __linux__
#	undef  ACE_PLATFORM_RPI
#	define ACE_PLATFORM_RPI 1
#elif  defined(__linux__)
#	undef  ACE_PLATFORM_LINUX
#	define ACE_PLATFORM_LINUX 1
#elif  defined(__ENVIRONMENT_IPHONE_OS_VERSION_MIN_REQUIRED__) \
    || defined(__ENVIRONMENT_TV_OS_VERSION_MIN_REQUIRED__)
#	undef  ACE_PLATFORM_IOS
#	define ACE_PLATFORM_IOS 1
#elif defined(__has_builtin) && __has_builtin(__is_target_os) && __is_target_os(xros)
#	undef  ACE_PLATFORM_VISIONOS
#	define ACE_PLATFORM_VISIONOS 1
#elif defined(__ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__)
#	undef  ACE_PLATFORM_OSX
#	define ACE_PLATFORM_OSX __ENVIRONMENT_MAC_OS_X_VERSION_MIN_REQUIRED__
#elif defined(__EMSCRIPTEN__)
#	include <emscripten/version.h>
#	undef  ACE_PLATFORM_EMSCRIPTEN
#	define ACE_PLATFORM_EMSCRIPTEN (__EMSCRIPTEN_major__ * 10000 + __EMSCRIPTEN_minor__ * 100 + __EMSCRIPTEN_tiny__)
#elif defined(__ORBIS__)
#	undef  ACE_PLATFORM_PS4
#	define ACE_PLATFORM_PS4 1
#elif defined(__PROSPERO__)
#	undef  ACE_PLATFORM_PS5
#	define ACE_PLATFORM_PS5 1
#elif  defined(__FreeBSD__)        \
    || defined(__FreeBSD_kernel__) \
    || defined(__NetBSD__)         \
    || defined(__OpenBSD__)        \
    || defined(__DragonFly__)
#	undef  ACE_PLATFORM_BSD
#	define ACE_PLATFORM_BSD 1
#elif defined(__GNU__)
#	undef  ACE_PLATFORM_HURD
#	define ACE_PLATFORM_HURD 1
#elif defined(__NX__)
#	undef  ACE_PLATFORM_NX
#	define ACE_PLATFORM_NX 1
#elif defined(__HAIKU__)
#	undef  ACE_PLATFORM_HAIKU
#	define ACE_PLATFORM_HAIKU 1
#endif //

#if !ACE_CRT_NONE
// https://sourceforge.net/p/predef/wiki/Libraries/
#	if defined(__BIONIC__)
#		undef  ACE_CRT_BIONIC
#		define ACE_CRT_BIONIC 1
#	elif defined(__GLIBC__)
#		undef  ACE_CRT_GLIBC
#		define ACE_CRT_GLIBC (__GLIBC__ * 10000 + __GLIBC_MINOR__ * 100)
#	elif defined(__apple_build_version__) || defined(__ORBIS__) || defined(__EMSCRIPTEN__) || defined(__llvm__) || defined(__HAIKU__)
#		undef  ACE_CRT_LIBCXX
#		define ACE_CRT_LIBCXX 1
#	elif defined(__MINGW32__) || defined(__MINGW64__)
#		undef  ACE_CRT_MINGW
#		define ACE_CRT_MINGW 1
#	elif defined(_MSC_VER)
#		undef  ACE_CRT_MSVC
#		define ACE_CRT_MSVC 1
#	endif //
#endif // !ACE_CRT_NONE

///
#define ACE_PLATFORM_POSIX (0   \
    ||  ACE_PLATFORM_ANDROID    \
    ||  ACE_PLATFORM_BSD        \
    ||  ACE_PLATFORM_EMSCRIPTEN \
    ||  ACE_PLATFORM_HAIKU      \
    ||  ACE_PLATFORM_HURD       \
    ||  ACE_PLATFORM_IOS        \
    ||  ACE_PLATFORM_LINUX      \
    ||  ACE_PLATFORM_NX         \
    ||  ACE_PLATFORM_OSX        \
    ||  ACE_PLATFORM_PS4        \
    ||  ACE_PLATFORM_PS5        \
    ||  ACE_PLATFORM_RPI        \
    ||  ACE_PLATFORM_VISIONOS   \
    )

///
#define ACE_PLATFORM_NONE !(0   \
    ||  ACE_PLATFORM_ANDROID    \
    ||  ACE_PLATFORM_BSD        \
    ||  ACE_PLATFORM_EMSCRIPTEN \
    ||  ACE_PLATFORM_HAIKU      \
    ||  ACE_PLATFORM_HURD       \
    ||  ACE_PLATFORM_IOS        \
    ||  ACE_PLATFORM_LINUX      \
    ||  ACE_PLATFORM_NX         \
    ||  ACE_PLATFORM_OSX        \
    ||  ACE_PLATFORM_PS4        \
    ||  ACE_PLATFORM_PS5        \
    ||  ACE_PLATFORM_RPI        \
    ||  ACE_PLATFORM_VISIONOS   \
    ||  ACE_PLATFORM_WINDOWS    \
    ||  ACE_PLATFORM_WINRT      \
    ||  ACE_PLATFORM_XBOXONE    \
    )

///
#define ACE_PLATFORM_OS_CONSOLE  (0 \
    ||  ACE_PLATFORM_NX             \
    ||  ACE_PLATFORM_PS4            \
    ||  ACE_PLATFORM_PS5            \
    ||  ACE_PLATFORM_WINRT          \
    ||  ACE_PLATFORM_XBOXONE        \
    )

///
#define ACE_PLATFORM_OS_DESKTOP  (0 \
    ||  ACE_PLATFORM_BSD            \
    ||  ACE_PLATFORM_HAIKU          \
    ||  ACE_PLATFORM_HURD           \
    ||  ACE_PLATFORM_LINUX          \
    ||  ACE_PLATFORM_OSX            \
    ||  ACE_PLATFORM_WINDOWS        \
    )

///
#define ACE_PLATFORM_OS_EMBEDDED (0 \
    ||  ACE_PLATFORM_RPI            \
    )

///
#define ACE_PLATFORM_OS_MOBILE   (0 \
    ||  ACE_PLATFORM_ANDROID        \
    ||  ACE_PLATFORM_IOS            \
    )

///
#define ACE_PLATFORM_OS_WEB      (0 \
    ||  ACE_PLATFORM_EMSCRIPTEN     \
    )

///
#if ACE_COMPILER_GCC
#	define ACE_COMPILER_NAME "GCC "       \
        ACE_STRINGIZE(__GNUC__) "."       \
        ACE_STRINGIZE(__GNUC_MINOR__) "." \
        ACE_STRINGIZE(__GNUC_PATCHLEVEL__)
#elif ACE_COMPILER_CLANG
#	define ACE_COMPILER_NAME "Clang "      \
        ACE_STRINGIZE(__clang_major__) "." \
        ACE_STRINGIZE(__clang_minor__) "." \
        ACE_STRINGIZE(__clang_patchlevel__)
#elif ACE_COMPILER_MSVC
#	if ACE_COMPILER_MSVC >= 1930 // Visual Studio 2022
#		define ACE_COMPILER_NAME "MSVC 17.0"
#	elif ACE_COMPILER_MSVC >= 1920 // Visual Studio 2019
#		define ACE_COMPILER_NAME "MSVC 16.0"
#	elif ACE_COMPILER_MSVC >= 1910 // Visual Studio 2017
#		define ACE_COMPILER_NAME "MSVC 15.0"
#	elif ACE_COMPILER_MSVC >= 1900 // Visual Studio 2015
#		define ACE_COMPILER_NAME "MSVC 14.0"
#	elif ACE_COMPILER_MSVC >= 1800 // Visual Studio 2013
#		define ACE_COMPILER_NAME "MSVC 12.0"
#	elif ACE_COMPILER_MSVC >= 1700 // Visual Studio 2012
#		define ACE_COMPILER_NAME "MSVC 11.0"
#	elif ACE_COMPILER_MSVC >= 1600 // Visual Studio 2010
#		define ACE_COMPILER_NAME "MSVC 10.0"
#	elif ACE_COMPILER_MSVC >= 1500 // Visual Studio 2008
#		define ACE_COMPILER_NAME "MSVC 9.0"
#	else
#		define ACE_COMPILER_NAME "MSVC"
#	endif //
#endif // ACE_COMPILER_

#if ACE_PLATFORM_ANDROID
#	define ACE_PLATFORM_NAME "Android " \
        ACE_STRINGIZE(ACE_PLATFORM_ANDROID)
#elif ACE_PLATFORM_BSD
#	define ACE_PLATFORM_NAME "BSD"
#elif ACE_PLATFORM_EMSCRIPTEN
#	define ACE_PLATFORM_NAME "Emscripten "      \
        ACE_STRINGIZE(__EMSCRIPTEN_major__) "." \
        ACE_STRINGIZE(__EMSCRIPTEN_minor__) "." \
        ACE_STRINGIZE(__EMSCRIPTEN_tiny__)
#elif ACE_PLATFORM_HAIKU
#	define ACE_PLATFORM_NAME "Haiku"
#elif ACE_PLATFORM_HURD
#	define ACE_PLATFORM_NAME "Hurd"
#elif ACE_PLATFORM_IOS
#	define ACE_PLATFORM_NAME "iOS"
#elif ACE_PLATFORM_LINUX
#	define ACE_PLATFORM_NAME "Linux"
#elif ACE_PLATFORM_NONE
#	define ACE_PLATFORM_NAME "None"
#elif ACE_PLATFORM_NX
#	define ACE_PLATFORM_NAME "NX"
#elif ACE_PLATFORM_OSX
#	define ACE_PLATFORM_NAME "macOS"
#elif ACE_PLATFORM_PS4
#	define ACE_PLATFORM_NAME "PlayStation 4"
#elif ACE_PLATFORM_PS5
#	define ACE_PLATFORM_NAME "PlayStation 5"
#elif ACE_PLATFORM_RPI
#	define ACE_PLATFORM_NAME "RaspberryPi"
#elif ACE_PLATFORM_VISIONOS
#	define ACE_PLATFORM_NAME "visionOS"
#elif ACE_PLATFORM_WINDOWS
#	define ACE_PLATFORM_NAME "Windows"
#elif ACE_PLATFORM_WINRT
#	define ACE_PLATFORM_NAME "WinRT"
#elif ACE_PLATFORM_XBOXONE
#	define ACE_PLATFORM_NAME "Xbox One"
#else
#	error "Unknown ACE_PLATFORM!"
#endif // ACE_PLATFORM_

#if ACE_CPU_ARM
#	define ACE_CPU_NAME "ARM"
#elif ACE_CPU_JIT
#	define ACE_CPU_NAME "JIT-VM"
#elif ACE_CPU_MIPS
#	define ACE_CPU_NAME "MIPS"
#elif ACE_CPU_PPC
#	define ACE_CPU_NAME "PowerPC"
#elif ACE_CPU_RISCV
#	define ACE_CPU_NAME "RISC-V"
#elif ACE_CPU_X86
#	define ACE_CPU_NAME "x86"
#endif // ACE_CPU_

#if ACE_CRT_BIONIC
#	define ACE_CRT_NAME "Bionic libc"
#elif ACE_CRT_GLIBC
#	define ACE_CRT_NAME "GNU C Library"
#elif ACE_CRT_MSVC
#	define ACE_CRT_NAME "MSVC C Runtime"
#elif ACE_CRT_MINGW
#	define ACE_CRT_NAME "MinGW C Runtime"
#elif ACE_CRT_LIBCXX
#	define ACE_CRT_NAME "Clang C Library"
#elif ACE_CRT_NEWLIB
#	define ACE_CRT_NAME "Newlib"
#elif ACE_CRT_NONE
#	define ACE_CRT_NAME "None"
#else
#	define ACE_CRT_NAME "Unknown CRT"
#endif // ACE_CRT_

#if ACE_ARCH_32BIT
#	define ACE_ARCH_NAME "32-bit"
#elif ACE_ARCH_64BIT
#	define ACE_ARCH_NAME "64-bit"
#endif // ACE_ARCH_

#if defined(__cplusplus)
#	if ACE_COMPILER_MSVC && defined(_MSVC_LANG) && _MSVC_LANG != __cplusplus
#		error "When using MSVC you must set /Zc:__cplusplus compiler option."
#	endif // ACE_COMPILER_MSVC && defined(_MSVC_LANG) && _MSVC_LANG != __cplusplus

#	if   __cplusplus < ACE_LANGUAGE_CPP17
#		define ACE_CPP_NAME "C++Unsupported"
#	elif __cplusplus < ACE_LANGUAGE_CPP20
#		define ACE_CPP_NAME "C++17"
#	elif __cplusplus < ACE_LANGUAGE_CPP23
#		define ACE_CPP_NAME "C++20"
#	else
// See: https://gist.github.com/bkaradzic/2e39896bc7d8c34e042b#orthodox-c
#		define ACE_CPP_NAME "C++WayTooModern"
#	endif // ACE_CPP_NAME
#else
#	define ACE_CPP_NAME "C++Unknown"
#endif // defined(__cplusplus)

#if defined(__cplusplus)

static_assert(__cplusplus >= ACE_LANGUAGE_CPP17, "\n\n"
    "\t** IMPORTANT! **\n\n"
    "\tC++17 standard support is required to build.\n"
    "\t\n");

// https://releases.llvm.org/
static_assert(!ACE_COMPILER_CLANG || ACE_COMPILER_CLANG >= 110000, "\n\n"
    "\t** IMPORTANT! **\n\n"
    "\tMinimum supported Clang version is 11.0 (October 12, 2020).\n"
    "\t\n");

// https://gcc.gnu.org/releases.html
static_assert(!ACE_COMPILER_GCC || ACE_COMPILER_GCC >= 80400, "\n\n"
    "\t** IMPORTANT! **\n\n"
    "\tMinimum supported GCC version is 8.4 (March 4, 2020).\n"
    "\t\n");

static_assert(!ACE_CPU_ENDIAN_BIG, "\n\n"
    "\t** IMPORTANT! **\n\n"
    "\tThe code was not tested for big endian, and big endian CPU is considered unsupported.\n"
    "\t\n");

static_assert(!ACE_PLATFORM_BSD || !ACE_PLATFORM_HAIKU || !ACE_PLATFORM_HURD, "\n\n"
    "\t** IMPORTANT! **\n\n"
    "\tYou're compiling for unsupported platform!\n"
    "\tIf you wish to support this platform, make your own fork, and modify code for _yourself_.\n"
    "\t\n"
    "\tDo not submit PR to main repo, it won't be considered, and it would code rot anyway. I have no ability\n"
    "\tto test on these platforms, and over years there wasn't any serious contributor who wanted to take\n"
    "\tburden of maintaining code for these platforms.\n"
    "\t\n");

#endif // defined(__cplusplus)

