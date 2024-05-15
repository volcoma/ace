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

// OS utils. Here is where the fun starts... good luck

#define ACE_QUOTE(...)     #__VA_ARGS__
#define ACE_COMMENT(...)   ACE_NO
#define ACE_UNCOMMENT(...) ACE_YES

#define ACE_YES(...) __VA_ARGS__
#define ACE_NO(...)

#define ACE_ON(v) (0 v(+1)) // usage: #if ACE_ON(ACE_COMPILER_MSVC)
#define ACE_IS    ACE_ON    // usage: #if ACE_ON(ACE_DEBUG)
#define ACE_HAS(...)                                                                                                   \
    ACE_COMPILER_CLANG(__has_feature(__VA_ARGS__))                                                                     \
    ACE_COMPILER_CELSE(__VA_ARGS__) // usage: #if ACE_HAS(cxx_exceptions)

#if defined(_WIN32)
#define ACE_PLATFORM_WINDOWS ACE_YES
#else
#define ACE_PLATFORM_WINDOWS ACE_NO
#endif

#ifdef __APPLE__
#define ACE_PLATFORM_APPLE ACE_YES

#ifdef TARGET_OS_IPHONE
// iOS or iOS Simulator
#define ACE_PLATFORM_APPLE_IOS ACE_YES
#else
// macOS
#define ACE_PLATFORM_APPLE_MAC ACE_YES
#endif

#else
#define ACE_PLATFORM_APPLE ACE_NO
#endif

#ifdef __linux__
#define ACE_PLATFORM_LINUX ACE_YES
#else
#define ACE_PLATFORM_LINUX ACE_NO
#endif

#ifdef __ANDROID__
#define ACE_PLATFORM_ANDROID ACE_YES
#else
#define ACE_PLATFORM_ANDROID ACE_NO
#endif

// Compiler utils
#if INTPTR_MAX == INT64_MAX
#define ACE_ARCH_64 ACE_YES
#define ACE_ARCH_32 ACE_NO
#else
#define ACE_ARCH_64 ACE_NO
#define ACE_ARCH_32 ACE_YES
#endif

#if defined(NDEBUG) || defined(_NDEBUG) || defined(RELEASE)
#define ACE_DEBUG   ACE_YES
#define ACE_RELEASE ACE_NO
#else
#define ACE_DEBUG   ACE_NO
#define ACE_RELEASE ACE_YES
#endif

#if defined(NDEVEL) || defined(_NDEVEL) || defined(PUBLIC)
#define ACE_PUBLIC  ACE_YES
#define ACE_DEVELOP ACE_NO
#else
#define ACE_PUBLIC  ACE_NO
#define ACE_DEVELOP ACE_YES
#endif

#if defined(__GNUC__) || defined(__MINGW32__)
#define ACE_COMPILER_GNUC ACE_YES
#else
#define ACE_COMPILER_GNUC ACE_NO
#endif

#if defined(__MINGW32__)
#define ACE_COMPILER_MINGW ACE_YES
#else
#define ACE_COMPILER_MINGW ACE_NO
#endif

#ifdef _MSC_VER
#define ACE_COMPILER_MSVC ACE_YES
#else
#define ACE_COMPILER_MSVC ACE_NO
#endif

#ifdef __clang__
#define ACE_COMPILER_CLANG ACE_YES
#else
#define ACE_COMPILER_CLANG ACE_NO
#endif

#if ACE_ON(ACE_COMPILER_MSVC) || ACE_ON(ACE_COMPILER_GNUC) || ACE_ON(ACE_COMPILER_CLANG) || ACE_ON(ACE_COMPILER_MINGW)
#define ACE_UNDEFINED_COMPILER ACE_NO
#else
#define ACE_UNDEFINED_COMPILER ACE_YES
#endif

#if ACE_ON(ACE_PLATFORM_WINDOWS) || ACE_ON(ACE_PLATFORM_LINUX) || ACE_ON(ACE_PLATFORM_APPLE) ||                        \
    ACE_ON(ACE_PLATFORM_ANDROID)
#define ACE_UNDEFINED_OS ACE_NO
#else
#define ACE_UNDEFINED_OS ACE_YES
#endif

template<bool>
inline bool eval()
{
    return true;
}

template<>
inline bool eval<false>()
{
    return false;
}
#define runtime_eval(_x) eval<!!(_x)>()
