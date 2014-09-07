#ifndef WIN_HPP
#define WIN_HPP

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

#ifdef MT
#include <process.h>
#endif


#endif