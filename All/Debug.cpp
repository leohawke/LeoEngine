#include "IndePlatform\ldef.h"

#include "Win.hpp"
#include "Debug.hpp"
#include <stdio.h>
#include <string.h>

namespace leo
{
	void ReSetStdIOtoFile(which_stdio which, wchar_t * filename)
	{
		switch (which)
		{
		case leo::which_stdio::istream:
			_wfreopen(filename, L"r", stdin);
			break;
		case leo::which_stdio::ostream:
			_wfreopen(filename, L"w", stdout);
			break;
		case leo::which_stdio::estream:
			_wfreopen(filename, L"w", stderr);
			break;
		default:
			break;
		}
	}
#if defined(_DEBUG) || defined (DEBUG)
	debug debug::global_debug;
	void debug::operator()(const wchar_t * strformat, ...)
	{
		const size_t count = 4096 / sizeof(wchar_t);
		wchar_t strBuffer[count] = { 0 };
		wchar_t *p = strBuffer;
		va_list vlArgs;
		va_start(vlArgs, strformat);
		vswprintf(strBuffer, count-1, strformat, vlArgs);
		va_end(vlArgs);
		static HANDLE consoleout = CreateFileW(L"CONOUT$", GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
		switch (which)
		{
		case leo::debug::which_output::CONSOLE:
			WriteConsoleW(consoleout, strBuffer,lstrlenW(strBuffer), nullptr, nullptr);
			break;
		case leo::debug::which_output::STD:
			wprintf(strBuffer);
			break;
		case leo::debug::which_output::VS:
			OutputDebugStringW(strBuffer);
			break;
		default:
			break;
		}
	}

	void debug::operator()(const char * strformat, ...)
	{
		const size_t count = 4096 / sizeof(char);
		char strBuffer[count] = { 0 };
		char *p = strBuffer;
		va_list vlArgs;
		va_start(vlArgs, strformat);
		vsnprintf(strBuffer,count-1,strformat, vlArgs);
		va_end(vlArgs);
		static HANDLE consoleout = CreateFileA("CONOUT$", GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
		switch (which)
		{
		case leo::debug::which_output::CONSOLE:
			WriteConsoleA(consoleout, strBuffer,static_cast<DWORD>(strlen(strBuffer)), nullptr, nullptr);
			break;
		case leo::debug::which_output::STD:
			printf(strBuffer);
			break;
		case leo::debug::which_output::VS:
			OutputDebugStringA(strBuffer);
			break;
		default:
			break;
		}
	}
#endif
	namespace win
	{
#if defined(_DEBUG) || defined (DEBUG)
			void OutputDebugLastError()
			{
				PVOID lpMsgBuf;
				FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, nullptr, GetLastError(),
					MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&lpMsgBuf, 0, nullptr);
				DebugPrintf(L"ErrorCode: %d, :%s", GetLastError(), lpMsgBuf);
				LocalFree(lpMsgBuf);
			}
#endif
	}
}