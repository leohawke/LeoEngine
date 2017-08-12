#include "Debug.h"
#include "NativeAPI.h"
#include <cstring>
#include <cerrno>
#include <cstdarg>
#if LFL_Win32
#	include "../Win32/LCLib/NLS.h"
#endif

namespace platform
{

	void
		terminate() lnothrow
	{
		std::abort();
	}

	int
		usystem(const char* cmd)
	{
#if LFL_Win32
		return ::_wsystem(platform_ex::Windows::MBCSToWCS(cmd, CP_UTF8).c_str());
#else
		return std::system(cmd);
#endif
	}

} // namespace platform;