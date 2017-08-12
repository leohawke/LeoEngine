#include "FileSystem.h"
#include "FFileIO.h"
#include "NativeAPI.h"
#include "../CHRLib/CharacterProcessing.h"
#include <LBase/ctime.h>

//	leo::is_time_no_leap_valid;
#if LFL_Win32
#	include "../Win32/LCLib/Mingw32.h"
//	platform_ex::ResolveReparsePoint, platform_ex::DirectoryFindData;
#	include <time.h> // for ::localtime_s;

namespace
{

	namespace LCL_Impl_details
	{
		// NOTE: To avoid hiding of global name, the declarations shall not be under
		//	namespace %platform.
		LCL_DeclW32Call(CreateSymbolicLinkW, kernel32, unsigned char, const wchar_t*, \
			const wchar_t*, unsigned long)
			using platform::wcast;

		// NOTE: As %SYMBOLIC_LINK_FLAG_DIRECTORY, but with correct type.
		lconstexpr const auto SymbolicLinkFlagDirectory(1UL);

		inline PDefH(void, W32_CreateSymbolicLink, const char16_t* dst,
			const char16_t* src, unsigned long flags)
			ImplExpr(LCL_CallF_Win32(CreateSymbolicLinkW, wcast(dst), wcast(src), flags))

	} // namespace LCL_Impl_details;

} // unnamed namespace;

using platform_ex::MakePathStringW;
using platform_ex::DirectoryFindData;
#else
#	include <dirent.h>
#	include <LBase/scope_guard.hpp> // for leo::swap_guard;
#	include <time.h> // for ::localtime_r;

using platform_ex::MakePathStringU;
#endif

using namespace CHRLib;
