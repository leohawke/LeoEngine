#include "sdef.h"
#include <LFramework/LCLib/Platform.h>

#ifdef LS_BUILD_DLL
#ifdef LFL_Win32
#include <Windows.h>
BOOL APIENTRY DllMain(HANDLE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	return TRUE;
}

#endif
#endif