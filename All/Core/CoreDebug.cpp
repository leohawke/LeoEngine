#include "platform.h"
#include "CoreDebug.hpp"


#if defined(DEBUG)
	void leo::win::OutputDebugLastError() {
		PVOID lpMsgBuf;
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, nullptr, GetLastError(),
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&lpMsgBuf, 0, nullptr);
		DebugPrintf(L"ErrorCode: %d, :%s", GetLastError(), lpMsgBuf);
		LocalFree(lpMsgBuf);
	}
#endif