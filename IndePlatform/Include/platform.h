#ifndef IndePlatform_platform_h
#define IndePlatform_platform_h

#include "platform_macro.h"


#ifdef PLATFORM_WIN32
#ifndef ARCH_XENON
#undef STRICT
#define STRICT
#ifndef NOMINMAX
#define NOMINMAX
#include <Windows.h>
#include <windowsx.h>
#undef NOMINMAX
#else
#include <Windows.h>
#include <windowsx.h>
#endif

#endif

#ifdef max
#undef min
#undef max
#endif

#ifdef MULTI_THREAD
#include <process.h>
#endif
#endif


#endif