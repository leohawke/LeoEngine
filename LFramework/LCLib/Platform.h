/*!	\file Platform.h
\ingroup Framework
\brief ͨ��ƽ̨�����ļ���
*/

#ifndef FrameWork_Platform_h
#define FrameWork_Platform_h 1

#include <LBase/ldef.h> // for std::uintmax_t, stdex::byte, stdex::octet,
//  ::ptrdiff_t, ::size_t, ::wint_t stdex::nullptr_t;

//@{
/*!
\brief Win32 ƽ̨��
\note ע�� _WIN32 ��Ԥ���壬��û�� _WIN64 ��
*/
#define LB_Platform_Win32 0x6100

/*!
\brief Win32 x86_64 ƽ̨��
\note ͨ�� Win64 ƽ̨��ע�� Win32 ָ Windows32 ��ϵͳ��������ϵ�ṹ���ơ�
\note ע�� _WIN32 �� _WIN64 ��ͬʱԤ���塣
*/
#define LB_Platform_Win64 0x6110

/*!
\brief MinGW32 ƽ̨��
\note ע�� MinGW-w64 �� MinGW.org ͬʱԤ������ __MINGW32__ ����û�� __MINGW64__ ��
*/
#define LB_Platform_MinGW32 0x6102

/*!
\brief MinGW-w64 x86_64 ƽ̨��
\note �� Win64 �������ݵ� MinGW32 ƽ̨��ע�� MinGW32 ָϵͳ���ƣ�������ϵ�ṹ���ơ�
\note ע�� MinGW-w64 �� x86_64 ��ͬʱԤ������ __MINGW32__ �� __MINGW64__ ��
*/
#define LB_Platform_MinGW64 0x6112


/*!
\def LB_Platform
\brief Ŀ��ƽ̨��
\note ע��˳��
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
//��ǰĬ���� Win32 ��ΪĿ��ƽ̨��
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

/*!
\brief X86 �ܹ���
*/
#define LB_Arch_X86 0x70086

/*!
\brief AMD64 �ܹ���
*/
#define LB_Arch_X64 0x76486

/*!
\brief ARM �ܹ���
*/
#define LB_Arch_ARM 0x70000

#ifdef LB_IMPL_MSCPP
//Visual C++
#if defined(_M_IX86)
#define LB_Arch LB_Arch_X86
#elif defined(_M_AMD64)
#define LB_Arch LB_Arch_X64
#elif defined(_M_ARM)
#define LB_Arch LB_Arch_ARM
#else
#error "unsupprot arch"
#endif
#else
//g++ clang++
#if defined(_X86_)
#define LB_Arch LB_Arch_X86
#elif defined(__amd64)
#define LB_Arch LB_Arch_X64
#endif
#endif

#if LB_Arch != LB_Arch_ARM
#define LB_SSE 1
#else
#define LB_NEON 1
#endif


namespace platform
{
	//! \since build 1.4
	inline namespace basic_types
	{
		/*!
		\brief ƽ̨ͨ���������͡�
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


