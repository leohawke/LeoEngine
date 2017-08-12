/*!	\file NativeAPI.h
\ingroup Framework
\brief 通用平台应用程序接口描述。
*/

#ifndef FrameWork_NativeAPI_h
#define FrameWork_NativeAPI_h 1

#include <LFramework/LCLib/Platform.h>
#include <LBase/type_op.hpp>
#include <LBase/lmacro.h>

#ifndef  LF_Platform
#error "Unknown platform found."
#endif // ! LF_Platform

namespace platform
{
#	if LFL_Win32
	using ssize_t = int;
#	else
	using ssize_t = leo::make_signed_t<std::size_t>;
#	endif
} // namespace platform;

static_assert(std::is_signed<platform::ssize_t>(),
	"Invalid signed size type found.");

#if LFL_Win32

#	ifndef UNICODE
#		define UNICODE 1
#	endif

#	ifndef WINVER
#		define WINVER _WIN32_WINNT_WIN7
#	endif

#	ifndef WIN32_LEAN_AND_MEAN
#		define WIN32_LEAN_AND_MEAN 1
#	endif

#	ifndef NOMINMAX
#		define NOMINMAX 1
#	endif

#	include <Windows.h>
#	include <direct.h>

//! \ingroup name_collision_workarounds
//@{
#	undef CopyFile
#	undef CreateHardLink
#	undef CreateSymbolicLink
#	undef DialogBox
#	undef DrawText
#	undef ExpandEnvironmentStrings
#	undef FindWindow
#	undef FormatMessage
#	undef GetMessage
#	undef GetObject
#	undef PostMessage
//@}

extern "C"
{
#	if defined(__MINGW32__) && !defined(__MINGW64_VERSION_MAJOR)
	LB_API struct ::tm* __cdecl __MINGW_NOTHROW
		_gmtime32(const ::__time32_t*);
#	endif

} // extern "C";
#endif

#if LFL_Win32 || LFL_API_POSIXFileSystem
#	include <sys/stat.h>

namespace platform
{

	enum class Mode
#	if LFL_Win32
		: unsigned short
#	else
		: ::mode_t
#	endif
	{
#	if LFL_Win32
		FileType = _S_IFMT,
		Directory = _S_IFDIR,
		Character = _S_IFCHR,
		FIFO = _S_IFIFO,
		Regular = _S_IFREG,
		UserRead = _S_IREAD,
		UserWrite = _S_IWRITE,
		UserExecute = _S_IEXEC,
		GroupRead = _S_IREAD >> 3,
		GroupWrite = _S_IWRITE >> 3,
		GroupExecute = _S_IEXEC >> 3,
		OtherRead = _S_IREAD >> 6,
		OtherWrite = _S_IWRITE >> 6,
		OtherExecute = _S_IEXEC >> 6,
#	else
		FileType = S_IFMT,
		Directory = S_IFDIR,
		Character = S_IFCHR,
		Block = S_IFBLK,
		Regular = S_IFREG,
		Link = S_IFLNK,
		Socket = S_IFSOCK,
		FIFO = S_IFIFO,
		UserRead = S_IRUSR,
		UserWrite = S_IWUSR,
		UserExecute = S_IXUSR,
		GroupRead = S_IRGRP,
		GroupWrite = S_IWGRP,
		GroupExecute = S_IXGRP,
		OtherRead = S_IROTH,
		OtherWrite = S_IWOTH,
		OtherExecute = S_IXOTH,
#	endif
		UserReadWrite = UserRead | UserWrite,
		User = UserReadWrite | UserExecute,
		GroupReadWrite = GroupRead | GroupWrite,
		Group = GroupReadWrite | GroupExecute,
		OtherReadWrite = OtherRead | OtherWrite,
		Other = OtherReadWrite | OtherExecute,
		Read = UserRead | GroupRead | OtherRead,
		Write = UserWrite | GroupWrite | OtherWrite,
		Execute = UserExecute | GroupExecute | OtherExecute,
		ReadWrite = Read | Write,
		Access = ReadWrite | Execute,
		//@{
#	if !LFL_Win32
		SetUserID = S_ISUID,
		SetGroupID = S_ISGID,
#	else
		SetUserID = 0,
		SetGroupID = 0,
#	endif
#	if LFL_Linux || _XOPEN_SOURCE
		VTX = S_ISVTX,
#	else
		VTX = 0,
#	endif
		PMode = SetUserID | SetGroupID | VTX | Access,
		All = PMode | FileType
		//@}
	};

	//! \relates Mode
	//@{
	DefBitmaskEnum(Mode)

#ifdef LB_IMPL_MSCPP
#pragma warning(push)
#pragma warning(disable:4800)
#endif
		inline PDefH(bool, HasExtraMode, Mode m)
		ImplRet(bool(m & ~(Mode::Access | Mode::FileType)))
		//@}
#ifdef LB_IMPL_MSCPP
#pragma warning(pop)
#endif
} // namespace platform;
#endif


#endif
