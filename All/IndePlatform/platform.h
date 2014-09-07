#ifndef IndePlatform_platform_h
#define IndePlatform_platform_h

#ifdef WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#ifndef NOMINMAX
#define NOMINMAX
#endif

#ifndef XENON
#define STRICT
#include <Windows.h>
#include <windowsx.h>
#endif

#ifdef _MT
#include <process.h>
#endif
#endif


#endif