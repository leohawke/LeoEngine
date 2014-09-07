#pragma once
namespace leo
{
	enum class which_stdio { istream, ostream, estream };
	void ReSetStdIOtoFile(which_stdio which, wchar_t * filename);
#if defined(_DEBUG) || defined (DEBUG)
	struct debug
	{
		enum class which_output {CONSOLE,STD,VS} which = which_output::VS;
		void operator()(const wchar_t * strformat, ...);
		void operator()(const char * strformat, ...);
		static debug global_debug;
	};
#ifndef Switch_Debug_Macro
#define Switch_Debug_Macro
#define DebugToVS() leo::debug::global_debug.which = leo::debug::which_output::VS
#define DebugToConsole() leo::debug::global_debug.which = leo::debug::which_output::CONSOLE
#define DebugToStdOutput() leo::debug::global_debug.which = leo::debug::which_output::STD
#endif
#ifndef DebugPrintf
#define DebugPrintf leo::debug::global_debug
#endif
#endif

#ifndef LogPrintf
#if defined(_DEBUG) || defined(DEBUG)
#define LogPrintf DebugPrintf
#else
#define LogPrintf wprintf
#endif
#endif
	namespace win
	{
#if defined(_DEBUG) || defined (DEBUG)
#ifndef DebugLastError
			void OutputDebugLastError();
#define DebugLastError win::OutputDebugLastError
#endif
#ifndef DebugElapseTime
#define	DebugElapseTime(scope) {__int64 countsPerSec;\
	QueryPerformanceFrequency((LARGE_INTEGER*)&countsPerSec); \
	double mSecondPerCount = 1.0 / (double)countsPerSec; \
	__int64 PrevTime, CurrTime; \
	QueryPerformanceCounter((LARGE_INTEGER*)&PrevTime); \
	scope; \
	QueryPerformanceCounter((LARGE_INTEGER*)&CurrTime); \
	leo::DebugPrintf(L#scope); \
	leo::DebugPrintf(L" : %lfs\n", (CurrTime - PrevTime)*mSecondPerCount); }
#endif
#else
#ifndef DebugPrintf
#define DebugPrintf(...)				{}						
#endif
#ifndef DebugLastError
#define DebugLastError()				{}
#endif
#ifndef DebugElapseTime
#define DebugElapseTime(scope)			{}					
#endif
#ifndef Switch_Debug_Macro
#define Switch_Debug_Macro
#define DebugToVS() {}
#define DebugToConsole() {}
#define DebugToStdOutput() {}
#endif
#endif
		}
}