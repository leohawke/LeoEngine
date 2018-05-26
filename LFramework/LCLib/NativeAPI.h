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

//@{
/*!
\def LCL_ReservedGlobal
\brief 按实现修饰全局保留名称。
\see ISO C11 7.1.3 和 WG21 N4594 17.6.4.3 。
\see https://msdn.microsoft.com/en-us/library/ttcz0bys.aspx 。
\see https://msdn.microsoft.com/en-us/library/ms235384(v=vs.100).aspx#Anchor_0 。
*/
#if LFL_Win32
#	define LCL_ReservedGlobal(_n) _##_n
#else
#	define LCL_ReservedGlobal(_n) _n
#endif
//! \brief 调用按实现修饰的具有全局保留名称的函数。
#define LCL_CallGlobal(_n, ...) ::LCL_ReservedGlobal(_n)(__VA_ARGS__)
//@}

#include <stdio.h>
#include <fcntl.h>

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


/*!
\ingroup name_collision_workarounds
\see http://pubs.opengroup.org/onlinepubs/9699919799/functions/V2_chap02.html#tag_15_01 。
*/
//@{
//! \note 定义在 <stdio.h> 。
//@{
#undef ctermid
#undef dprintf
#undef fdopen
//! \note 已知 DS 平台的 newlib 可能使用宏。
#undef fileno
#undef flockfile
#undef fmemopen
#undef fseeko
#undef ftello
#undef ftrylockfile
#undef funlockfile
#undef getc_unlocked
#undef getchar_unlocked
#undef getdelim
#undef getline
#undef open_memstream
#undef pclose
#undef popen
#undef putc_unlocked
#undef putchar_unlocked
#undef renameat
#undef tempnam
#undef vdprintf
//@}
//! \note 定义在 <dirent.h> 。
//@{
#undef alphasort
#undef closedir
#undef dirfd
#undef fdopendir
#undef opendir
#undef readdir
#undef readdir_r
#undef rewinddir
#undef scandir
#undef seekdir
#undef telldir
//@}
//! \note 定义在 <fcntl.h> 。
//@{
#undef creat
#undef fcntl
#undef open
#undef openat
#undef posix_fadvise
#undef posix_fallocate
//@}
//! \note 定义在 <semaphore.h> 。
#undef sem_close
#undef sem_destroy
#undef sem_getvalue
#undef sem_init
#undef sem_open
#undef sem_post
#undef sem_timedwait
#undef sem_trywait
#undef sem_unlink
#undef sem_wait
//! \note 定义在 <unistd.h> 。
//@{
#undef _exit
#undef access
#undef alarm
#undef chdir
#undef chown
#undef close
#undef confstr
#undef crypt
#undef dup
#undef dup2
#undef encrypt
#undef execl
#undef execle
#undef execlp
#undef execv
#undef execve
#undef execvp
#undef faccessat
#undef fchdir
#undef fchown
#undef fchownat
#undef fdatasync
#undef fexecve
#undef fork
#undef fpathconf
#undef fsync
#undef ftruncate
#undef getcwd
#undef getegid
#undef geteuid
#undef getgid
#undef getgroups
#undef gethostid
#undef gethostname
#undef getlogin
#undef getlogin_r
#undef getopt
#undef getpgid
#undef getpgrp
#undef getpid
#undef getppid
#undef getsid
#undef getuid
#undef isatty
#undef lchown
#undef link
#undef linkat
#undef lockf
#undef lseek
#undef nice
#undef pathconf
#undef pause
#undef pipe
#undef pread
#undef pwrite
#undef read
#undef readlink
#undef readlinkat
#undef rmdir
#undef setegid
#undef seteuid
#undef setgid
#undef setpgid
#undef setpgrp
#undef setregid
#undef setreuid
#undef setsid
#undef setuid
#undef sleep
#undef swab
#undef symlink
#undef symlinkat
#undef sync
#undef sysconf
#undef tcgetpgrp
#undef tcsetpgrp
#undef truncate
#undef ttyname
#undef ttyname_r
#undef unlink
#undef unlinkat
#undef write
//@}
//! \note 定义在 <sys/mman.h> 。
//@{
#undef mlock
#undef mlockall
#undef mmap
#undef mprotect
#undef msync
#undef munlock
#undef munlockall
#undef munmap
#undef posix_madvise
#undef posix_mem_offset
#undef posix_typed_mem_get_info
#undef posix_typed_mem_open
#undef shm_open
#undef shm_unlink
//@}
//! \note 定义在 <sys/stat.h> 。
//@{
#undef chmod
#undef fchmod
#undef fchmodat
#undef fstat
#undef fstatat
#undef futimens
#undef lstat
#undef mkdir
#undef mkdirat
#undef mkfifo
#undef mkfifoat
#undef mknod
#undef mknodat
#undef stat
#undef umask
#undef utimensat
//@}
//@}


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
#	undef PeekMessage
#	undef GetMessage
#	undef GetObject
#	undef PostMessage
#	undef SetEnvironmentVariable
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
	\brief 取文件描述符对应的句柄。
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
		//! \brief 打开模式。
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
		//! \warning 库实现内部使用，需要特定的二进制支持。
		//@{
#if O_LARGEFILE
		//! \note 指定 64 位文件大小。
		YCL_Impl_OMode(LargeFile, LARGEFILE),
#else
		LargeFile = 0,
#endif
#if _O_OBTAIN_DIR
		YCL_Impl_OMode(ObtainDirectory, OBTAIN_DIR),
#else
		//! \note 设置 FILE_FLAG_BACKUP_SEMANTICS 。
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
	\note 第三参数表示是否跟随链接。
	\pre 间接断言：指针参数非空。
	\note DS 和 Win32 平台：忽略第三参数，始终不跟随链接。
	*/
	//@{
	/*!
	\brief 带检查的可跟随链接的 \c stat 调用。
	\throw std::system_error 检查失败。
	\note 最后一个参数表示调用签名。
	*/
	//@{
	LF_API LB_NONNULL(2, 4) void
		cstat(struct ::stat&, const char*, bool, const char*);
	LF_API LB_NONNULL(3) void
		cstat(struct ::stat&, int, const char*);
	//@}

	//! \brief 可跟随链接的 \c stat 调用。
	LF_API LB_NONNULL(2) int
		estat(struct ::stat&, const char*, bool) lnothrowv;
	inline PDefH(int, estat, struct ::stat& st, int fd) lnothrow
		ImplRet(::fstat(fd, &st))
		//@}

} // namespace platform_ex;
#endif


#endif
