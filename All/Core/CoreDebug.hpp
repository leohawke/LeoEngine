////////////////////////////////////////////////////////////////////////////
//
//  Leo Engine Source File.
//  Copyright (C), FNS Studios, 2014-2015.
// -------------------------------------------------------------------------
//  File name:   Core/CoreDebug.hpp
//  Version:     v1.00
//  Created:     05/16/2015 by leo hawke.
//  Compilers:   Visual Studio.NET 2015
//  Description: 一些依赖相关的调试打印
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////
#ifndef Core_CoreDebug_HPP
#define Core_CoreDebug_HPP

#include "debugoutput.hpp"

namespace leo {
	namespace win {
#if defined(DEBUG)
		void OutputDebugLastError();
#endif
	}
}

#if defined(DEBUG)
#define DebugOutputLastError leo::win::OutputDebugLastError
#else
#define DebugOutputLastError(...) {}
#endif

#if defined(DEBUG)
#ifndef DebugElapseTime
#define	DebugElapseTime(scope) {__int64 countsPerSec;\
	QueryPerformanceFrequency((LARGE_INTEGER*)&countsPerSec); \
	double mSecondPerCount = 1.0 / (double)countsPerSec; \
	__int64 PrevTime, CurrTime; \
	QueryPerformanceCounter((LARGE_INTEGER*)&PrevTime); \
	scope; \
	QueryPerformanceCounter((LARGE_INTEGER*)&CurrTime); \
	DebugPrintf(L#scope); \
	DebugPrintf(L" : %lfs\n", (CurrTime - PrevTime)*mSecondPerCount); }
#endif
#else
#define	DebugElapseTime(scope) {scope;}
#endif

#endif
