/*!	\file NativeAPI.h
\ingroup Framework
\brief ͨ��ƽ̨Ӧ�ó���ӿ�������
*/

#ifndef FrameWork_NativeAPI_h
#define FrameWork_NativeAPI_h 1

#include <LFramework/LCLib/Platform.h>
#include <LBase/type_op.hpp>
#include <LBase/lmacro.h>

#ifndef  LF_Platform
#error "Unknown platform found."
#endif // ! LF_Platform

//@{
/*!
\def LCL_ReservedGlobal
\brief ��ʵ������ȫ�ֱ������ơ�
\see ISO C11 7.1.3 �� WG21 N4594 17.6.4.3 ��
\see https://msdn.microsoft.com/en-us/library/ttcz0bys.aspx ��
\see https://msdn.microsoft.com/en-us/library/ms235384(v=vs.100).aspx#Anchor_0 ��
*/
#if LFL_Win32
#	define LCL_ReservedGlobal(_n) _##_n
#else
#	define LCL_ReservedGlobal(_n) _n
#endif
//! \brief ���ð�ʵ�����εľ���ȫ�ֱ������Ƶĺ�����
#define LCL_CallGlobal(_n, ...) ::LCL_ReservedGlobal(_n)(__VA_ARGS__)
//@}

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
#   include <io.h>

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


namespace platform_ex
{

	/*!
	\brief ȡ�ļ���������Ӧ�ľ����
	*/
	inline PDefH(::HANDLE, ToHandle, int fd) lnothrow
		ImplRet(::HANDLE(::_get_osfhandle(fd)))

} // namespace platform_ex;
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


		//@{
		//! \brief ��ģʽ��
		enum class OpenMode : int
	{
#define YCL_Impl_OMode(_n, _nm) _n = LCL_ReservedGlobal(O_##_nm)
#if LFL_Win32
#	define YCL_Impl_OMode_POSIX(_n, _nm) _n = 0
#	define YCL_Impl_OMode_Win32(_n, _nm) YCL_Impl_OMode(_n, _nm)
#else
#	define YCL_Impl_OMode_POSIX(_n, _nm) YCL_Impl_OMode(_n, _nm)
#	define YCL_Impl_OMode_Win32(_n, _nm) _n = 0
#endif
#if O_CLOEXEC
		YCL_Impl_OMode_POSIX(CloseOnExec, CLOEXEC),
#endif
		YCL_Impl_OMode(Create, CREAT),
#if O_DIRECTORY
		YCL_Impl_OMode_POSIX(Directory, DIRECTORY),
#else
		// FIXME: Platform %DS does not support this.
		Directory = 0,
#endif
		YCL_Impl_OMode(Exclusive, EXCL),
		CreateExclusive = Create | Exclusive,
		YCL_Impl_OMode_POSIX(NoControllingTerminal, NOCTTY),
#if O_NOFOLLOW
		YCL_Impl_OMode_POSIX(NoFollow, NOFOLLOW),
#else
		// NOTE: Platform %DS does not support links.
		// NOTE: Platform %Win32 does not support links for these APIs.
		NoFollow = 0,
#endif
		YCL_Impl_OMode(Truncate, TRUNC),
#if O_TTY_INIT
		YCL_Impl_OMode_POSIX(TerminalInitialize, TTY_INIT),
#endif
		YCL_Impl_OMode(Append, APPEND),
#if O_DSYNC
		YCL_Impl_OMode_POSIX(DataSynchronized, DSYNC),
#endif
		YCL_Impl_OMode_POSIX(Nonblocking, NONBLOCK),
#if O_RSYNC
		YCL_Impl_OMode_POSIX(ReadSynchronized, RSYNC),
#endif
		YCL_Impl_OMode_POSIX(Synchronized, SYNC),
#if O_EXEC
		YCL_Impl_OMode_POSIX(Execute, EXEC),
#endif
		YCL_Impl_OMode(Read, RDONLY),
		YCL_Impl_OMode(ReadWrite, RDWR),
#if O_SEARCH
		YCL_Impl_OMode_POSIX(Search, SEARCH),
#endif
		YCL_Impl_OMode(Write, WRONLY),
		ReadWriteAppend = ReadWrite | Append,
		ReadWriteTruncate = ReadWrite | Truncate,
		WriteAppend = Write | Append,
		WriteTruncate = Write | Truncate,
		YCL_Impl_OMode_Win32(Text, TEXT),
		YCL_Impl_OMode_Win32(Binary, BINARY),
		Raw = Binary,
		ReadRaw = Read | Raw,
		ReadWriteRaw = ReadWrite | Raw,
		// NOTE: On GNU/Hurd %O_ACCMODE can be zero.
#if O_ACCMODE
		YCL_Impl_OMode_POSIX(AccessMode, ACCMODE),
#else
		AccessMode = Read | Write | ReadWrite,
#endif
		YCL_Impl_OMode_Win32(WText, WTEXT),
		YCL_Impl_OMode_Win32(U16Text, U16TEXT),
		YCL_Impl_OMode_Win32(U8Text, U8TEXT),
		YCL_Impl_OMode_Win32(NoInherit, NOINHERIT),
		YCL_Impl_OMode_Win32(Temporary, TEMPORARY),
		YCL_Impl_OMode_Win32(ShortLived, SHORT_LIVED),
		CreateTemporary = Create | Temporary,
		CreateShortLived = Create | ShortLived,
		YCL_Impl_OMode_Win32(Sequential, SEQUENTIAL),
		YCL_Impl_OMode_Win32(Random, RANDOM),
#if O_NDELAY
		YCL_Impl_OMode(NoDelay, NDELAY),
#endif
		//! \warning ��ʵ���ڲ�ʹ�ã���Ҫ�ض��Ķ�����֧�֡�
		//@{
#if O_LARGEFILE
		//! \note ָ�� 64 λ�ļ���С��
		YCL_Impl_OMode(LargeFile, LARGEFILE),
#else
		LargeFile = 0,
#endif
#if _O_OBTAIN_DIR
		YCL_Impl_OMode(ObtainDirectory, OBTAIN_DIR),
#else
		//! \note ���� FILE_FLAG_BACKUP_SEMANTICS ��
		ObtainDirectory = Directory,
#endif
		//@}
		None = 0
#undef YCL_Impl_OMode_POSIX
#undef YCL_Impl_OMode_Win32
#undef YCL_Impl_OMode
	};

	//! \relates OpenMode
	DefBitmaskEnum(OpenMode)
		//@}
		//@}
#ifdef LB_IMPL_MSCPP
#pragma warning(pop)
#endif
} // namespace platform;

namespace platform_ex
{

	/*!
	\note ����������ʾ�Ƿ�������ӡ�
	\pre ��Ӷ��ԣ�ָ������ǿա�
	\note DS �� Win32 ƽ̨�����Ե���������ʼ�ղ��������ӡ�
	*/
	//@{
	/*!
	\brief �����Ŀɸ������ӵ� \c stat ���á�
	\throw std::system_error ���ʧ�ܡ�
	\note ���һ��������ʾ����ǩ����
	*/
	//@{
	LF_API LB_NONNULL(2, 4) void
		cstat(struct ::stat&, const char*, bool, const char*);
	LF_API LB_NONNULL(3) void
		cstat(struct ::stat&, int, const char*);
	//@}

	//! \brief �ɸ������ӵ� \c stat ���á�
	LF_API LB_NONNULL(2) int
		estat(struct ::stat&, const char*, bool) lnothrowv;
	inline PDefH(int, estat, struct ::stat& st, int fd) lnothrow
		ImplRet(::fstat(fd, &st))
		//@}

} // namespace platform_ex;
#endif


#endif
