#undef __STRICT_ANSI__
#include <LBase/FFileIO.h>
#include <LBase/Debug.h>
#include <LBase/FReference.h>
#include <LBase/NativeAPI.h>
#include <LBase/functional.hpp>

#if LFL_Win32
#	if defined(__MINGW32__) && !defined(__MINGW64_VERSION_MAJOR)
// At least one headers of <stdlib.h>, <stdio.h>, <Windows.h>, <Windef.h>
//	(and probably more) should have been included to make the MinGW-W64 macro
//	available if it is really being used.
#		undef _fileno
#	endif
#	include <LBase/Win32/Mingw32.h>
//	platform_ex::GetErrnoFromWin32, platform_ex::QueryFileLinks,
//	platform_ex::QueryFileNodeID, platform_ex::QueryFileTime,
//	platform_ex::WCSToUTF8, platform_ex::UTF8ToWCS, platform_ex::ConvertTime,
//	platform_ex::SetFileTime;

//using platform_ex::FileAttributes;
//using platform_ex::GetErrnoFromWin32;
//using platform_ex::QueryFileLinks;
////@{
//using platform_ex::QueryFileNodeID;
//using platform_ex::QueryFileTime;
using platform_ex::Windows::UTF8ToWCS;
using platform_ex::Windows::WCSToUTF8;
//using platform_ex::ConvertTime;
//@}
#elif LFL_API_POSIXFileSystem
#	include <LBase/CHRLib/CharacterProcessing.h>
#	include <dirent.h>

using namespace CHRLib;
#else
#	error "Unsupported platform found."
#endif

namespace platform
{
	// XXX: Catch %std::bad_alloc?
#define LFL_Impl_RetTryCatchAll(...) \
	TryRet(__VA_ARGS__) \
	CatchExpr(std::bad_alloc&, errno = ENOMEM) \
	CatchIgnore(...)
}

namespace platform
{
	int
		upclose(std::FILE* fp) lnothrowv
	{
		LAssertNonnull(fp);
#if LFL_Win32
		return ::_pclose(fp);
#else
		return ::pclose(fp);
#endif
	}

	std::FILE*
		upopen(const char* filename, const char* mode) lnothrowv
	{
		LAssertNonnull(filename);
		LAssert(Deref(mode) != char(), "Invalid argument found.");
#if LFL_Win32
		LFL_Impl_RetTryCatchAll(::_wpopen(UTF8ToWCS(filename).c_str(),
			UTF8ToWCS(mode).c_str()))
			return{};
#else
		return ::popen(filename, mode);
#endif
	}
	std::FILE*
		upopen(const char16_t* filename, const char16_t* mode) lnothrowv
	{
		LAssertNonnull(filename);
		LAssert(Deref(mode) != char(), "Invalid argument found.");
#if LFL_Win32
		return ::_wpopen(wcast(filename), wcast(mode));
#else
		LFL_Impl_RetTryCatchAll(::popen(MakeMBCS(filename).c_str(),
			MakeMBCS(mode).c_str()))
			return{};
#endif
	}

	ImplDeDtor(FileOperationFailure)
}
