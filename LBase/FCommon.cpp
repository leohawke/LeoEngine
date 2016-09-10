#include <LBase/Debug.h>
#include <LBase/NativeAPI.h>
#include <cstring>
#include <cerrno>
#include <cstdarg>
#if LFL_Win32
#	include <LBase/Win32/Mingw32.h>
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
		return ::_wsystem(platform_ex::MBCSToWCS(cmd, CP_UTF8).c_str());
#else
		return std::system(cmd);
#endif
	}

} // namespace platform;