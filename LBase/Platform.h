/*!	\file Platform.h
\ingroup Framework
\brief 通用平台描述文件。
*/

#ifndef FrameWork_Platform_h
#define FrameWork_Platform_h 1

#include <LBase/ldef.h> // for std::uintmax_t, stdex::byte, stdex::octet,
//  ::ptrdiff_t, ::size_t, ::wint_t stdex::nullptr_t;

//@{
/*!
\brief Win32 平台。
\note 注意 _WIN32 被预定义，但没有 _WIN64 。
*/
#define LB_Platform_Win32 0x6100

/*!
\brief Win32 x86_64 平台。
\note 通称 Win64 平台。注意 Win32 指 Windows32 子系统，并非体系结构名称。
\note 注意 _WIN32 和 _WIN64 被同时预定义。
*/
#define LB_Platform_Win64 0x6110

/*!
\brief MinGW32 平台。
\note 注意 MinGW-w64 和 MinGW.org 同时预定义了 __MINGW32__ ，但没有 __MINGW64__ 。
*/
#define LB_Platform_MinGW32 0x6102

/*!
\brief MinGW-w64 x86_64 平台。
\note 和 Win64 基本兼容的 MinGW32 平台。注意 MinGW32 指系统名称，并非体系结构名称。
\note 注意 MinGW-w64 在 x86_64 上同时预定义了 __MINGW32__ 和 __MINGW64__ 。
*/
#define LB_Platform_MinGW64 0x6112


/*!
\def LB_Platform
\brief 目标平台。
\note 注意顺序。
*/
#ifdef __MSYS__
#	error "MSYS is not currently supported. Use MinGW targets instead."
#elif defined(__MINGW64__)
#	define LB_Platform LB_Platform_MinGW64
#elif defined(__MINGW32__)
#	define LB_Platform LB_Platform_MinGW32
#elif defined(_WIN64)
#	define LB_Platform LB_Platform_Win64
#elif defined(_WIN32)
#	define LB_Platform LB_Platform_Win32
#elif defined(__ANDROID__)
// FIXME: Architecture detection.
#	define LB_Platform LB_Platform_Android_ARM
#elif defined(__linux__)
#	ifdef __i386__
#		define LB_Platform LB_Platform_Linux_x64
#	elif defined(__x86_64__)
#		define LB_Platform LB_Platform_Linux_x86
#	endif
#elif defined(__APPLE__)
#	ifdef __MACH__
#		define LB_Platform LB_Platform_OS_X
#	else
#		error "Apple platforms other than OS X is not supported."
#	endif
#elif !defined(LB_Platform)
//当前默认以 Win32 作为目标平台。
#	define LB_Platform LB_Platform_Win32
#endif
//@}

#if LB_Platform == LB_Platform_Win32
#	define LFL_Win32 1
#	define LB_Hosted 1
#elif LB_Platform == LB_Platform_MinGW32
#	define LFL_MinGW 1
#	define LFL_Win32 1
#	define LB_Hosted 1
#elif LB_Platform == LB_Platform_Win64
#	define LFL_Win32 1
#	define LFL_Win64 1
#	define LB_Hosted 1
#	ifndef LB_Use_POSIXThread
#		define LB_Use_POSIXThread 1
#	endif
#elif LB_Platform == LB_Platform_MinGW64
#	define LFL_MinGW 1
#	define LFL_Win32 1
#	define LFL_Win64 1
#	define LB_Hosted 1
#	ifndef LB_Use_POSIXThread
#		define LB_Use_POSIXThread 1
#	endif
#endif

#if __STDCPP_THREADS__
#	define LB_Multithread 1
#elif LFL_Win32 || LFL_Android || LFL_Linux || LFL_OS_X
#	define LB_Multithread 1
#else
#	define LB_Multithread 0
#endif

namespace platform
{
	//! \since build 1.4
	inline namespace basic_types
	{
		/*!
		\brief 平台通用数据类型。
		*/
		//@{
		using stdex::byte;
		using stdex::octet;
		using stdex::ptrdiff_t;
		using stdex::size_t;
		using stdex::wint_t;
		using stdex::nullptr_t;
		//@}

	} // inline namespace basic_types;

} // namespace platform;

namespace platform_ex {
	using namespace platform::basic_types;
}

#endif


