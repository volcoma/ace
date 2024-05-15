#pragma once

#include "config.hpp"

#include <thread>

// An attempt at making a wrapper to deal with many Linuxes as well as Windows. Please edit as needed.
#if ACE_ON(ACE_PLATFORM_WINDOWS)
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <processthreadsapi.h>

#ifdef min
#undef min
#endif

#ifdef max
#undef max
#endif

namespace platform
{

inline void set_thread_name(const char* threadName)
{
    // Convert to wide string
    wchar_t wname[64];
    mbstowcs(wname, threadName, sizeof(wname)/sizeof(wname[0]) - 1);
    SetThreadDescription(GetCurrentThread(), wname);
}
} // namespace platform
#elif ACE_ON(ACE_PLATFORM_LINUX)
#include <pthread.h>
namespace platform
{
inline void set_thread_name(const char* threadName)
{
    pthread_setname_np(phread_self(), threadName);
}
} // namespace platform
#elif ACE_ON(ACE_PLATFORM_APPLE)
#include <pthread.h>
namespace platform
{
inline void set_thread_name(const char* threadName)
{
    pthread_setname_np(threadName);
}
} // namespace platform
#else
namespace platform
{
inline void set_thread_name(const char* threadName)
{
}
} // namespace platform
#endif

