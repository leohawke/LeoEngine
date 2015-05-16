#include "platform.h"
#include "DebugOutput.hpp"
#include <stdio.h>
#include <string.h>

using namespace leo;

	void RedirectStdIO(stdio_type which, char * target)
	{
		switch (which)
		{
		case leo::stdio_type::istream:
			freopen(target,"r", stdin);
			break;
		case leo::stdio_type::ostream:
			freopen(target,"w", stdout);
			break;
		case leo::stdio_type::estream:
			freopen(target,"w", stderr);
			break;
		default:
			break;
		}
	}

#if defined (DEBUG)
	details::debug_helper details::debug_helper::global_debug;
	using namespace details;
	void debug_helper::operator()(const wchar_t * strformat, ...)
	{
		const size_t count = 4096 / sizeof(wchar_t);
		wchar_t strBuffer[count] = { 0 };
		wchar_t *p = strBuffer;
		va_list vlArgs;
		va_start(vlArgs, strformat);
		vswprintf(strBuffer, count - 1, strformat, vlArgs);
		va_end(vlArgs);
		static HANDLE consoleout = CreateFileW(L"CONOUT$", GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
		switch (which)
		{
#if defined(PLATFORM_WIN32)
		case debug_helper::output_type::console:
			WriteConsoleW(consoleout, strBuffer, lstrlenW(strBuffer), nullptr, nullptr);
			break;
#endif
		case debug_helper::output_type::std:
			wprintf(strBuffer);
			break;
#if defined(IDE_VS)
		case debug_helper::output_type::visualstudio:
			OutputDebugStringW(strBuffer);
			break;
#endif
		default:
			break;
		}
	}

	void debug_helper::operator()(const char * strformat, ...)
	{
		const size_t count = 4096 / sizeof(char);
		char strBuffer[count] = { 0 };
		char *p = strBuffer;
		va_list vlArgs;
		va_start(vlArgs, strformat);
		vsnprintf(strBuffer, count - 1, strformat, vlArgs);
		va_end(vlArgs);
		static HANDLE consoleout = CreateFileA("CONOUT$", GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
		switch (which)
		{
#if defined(PLATFORM_WIN32)
		case debug_helper::output_type::console:
			WriteConsoleA(consoleout, strBuffer, strlen(strBuffer), nullptr, nullptr);
			break;
#endif
		case debug_helper::output_type::std:
			printf(strBuffer);
			break;
#if defined(IDE_VS)
		case debug_helper::output_type::visualstudio:
			OutputDebugStringA(strBuffer);
			break;
#endif
		default:
			break;
		}
	}

	void debug_helper::operator()(record_level level,const char * strformat, ...)
	{
		const size_t count = 4096 / sizeof(char);
		char strBuffer[count] = { 0 };
		char *p = strBuffer;
		va_list vlArgs;
		va_start(vlArgs, strformat);
		vsnprintf(strBuffer, count - 1, strformat, vlArgs);
		va_end(vlArgs);
		static HANDLE consoleout = CreateFileA("CONOUT$", GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
		switch (which)
		{
#if defined(PLATFORM_WIN32)
		case debug_helper::output_type::console:
			WriteConsoleA(consoleout, strBuffer, strlen(strBuffer), nullptr, nullptr);
			break;
#endif
		case debug_helper::output_type::std:
			printf(strBuffer);
			break;
#if defined(IDE_VS)
		case debug_helper::output_type::visualstudio:
			OutputDebugStringA(strBuffer);
			break;
#endif
		default:
			break;
		}
	}
#endif
