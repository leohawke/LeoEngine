#include "NativeAPI.h"
#include "FFileIO.h"

#if LFL_Win32 || LCL_API_POSIXFileSystem
namespace platform_ex
{

	LF_API LB_NONNULL(2) void
		cstat(struct ::stat& st, const char* path, bool follow_link, const char* sig)
	{
		const int res(estat(st, path, follow_link));

		if (res < 0)
#if !(LFL_DS || LFL_Win32)
			LCL_Raise_SysE(, "::stat", sig);
#else
		{
			if (follow_link)
				LCL_Raise_SysE(, "::stat", sig);
			else
				LCL_Raise_SysE(, "::lstat", sig);
		}
#endif
	}
	void
		cstat(struct ::stat& st, int fd, const char* sig)
	{
		const int res(::fstat(fd, &st));

		if (res < 0)
			LCL_Raise_SysE(, "::stat", sig);
	}


	LB_NONNULL(2) int
		estat(struct ::stat& st, const char* path, bool follow_link) lnothrowv
	{
		using platform::Nonnull;

#	if LCL_DS || LFL_Win32
		lunused(follow_link);
		return ::stat(Nonnull(path), &st);
#	else
		return (follow_link ? ::stat : ::lstat)(Nonnull(path), &st);
#	endif
	}

} // namespace platform_ex;
#endif