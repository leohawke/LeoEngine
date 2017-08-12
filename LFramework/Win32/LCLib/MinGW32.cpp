#include "Mingw32.h"
#if LFL_Win32
#include "../../LCLib/FileSystem.h"
#include "Registry.h"
#include "../../Core/LCoreUtilities.h"
#include <LBase/container.hpp>
#include <LBase/scope_gurad.hpp>
#include "NLS.h"
using namespace leo;
using platform::NodeCategory;
#endif
namespace platform_ex {
#if LFL_Win32

	namespace Windows {
		int
			ConvertToErrno(ErrorCode err) lnothrow
		{
			// NOTE: This mapping is from universal CRT in Windows SDK 10.0.10150.0,
			//	ucrt/misc/errno.cpp, except for fix of the bug error 124: it shall be
			//	%ERROR_INVALID_LEVEL but not %ERROR_INVALID_HANDLE. See https://connect.microsoft.com/VisualStudio/feedback/details/1641428.
			switch (err)
			{
			case ERROR_INVALID_FUNCTION:
				return EINVAL;
			case ERROR_FILE_NOT_FOUND:
			case ERROR_PATH_NOT_FOUND:
				return ENOENT;
			case ERROR_TOO_MANY_OPEN_FILES:
				return EMFILE;
			case ERROR_ACCESS_DENIED:
				return EACCES;
			case ERROR_INVALID_HANDLE:
				return EBADF;
			case ERROR_ARENA_TRASHED:
			case ERROR_NOT_ENOUGH_MEMORY:
			case ERROR_INVALID_BLOCK:
				return ENOMEM;
			case ERROR_BAD_ENVIRONMENT:
				return E2BIG;
			case ERROR_BAD_FORMAT:
				return ENOEXEC;
			case ERROR_INVALID_ACCESS:
			case ERROR_INVALID_DATA:
				return EINVAL;
			case ERROR_INVALID_DRIVE:
				return ENOENT;
			case ERROR_CURRENT_DIRECTORY:
				return EACCES;
			case ERROR_NOT_SAME_DEVICE:
				return EXDEV;
			case ERROR_NO_MORE_FILES:
				return ENOENT;
			case ERROR_LOCK_VIOLATION:
				return EACCES;
			case ERROR_BAD_NETPATH:
				return ENOENT;
			case ERROR_NETWORK_ACCESS_DENIED:
				return EACCES;
			case ERROR_BAD_NET_NAME:
				return ENOENT;
			case ERROR_FILE_EXISTS:
				return EEXIST;
			case ERROR_CANNOT_MAKE:
			case ERROR_FAIL_I24:
				return EACCES;
			case ERROR_INVALID_PARAMETER:
				return EINVAL;
			case ERROR_NO_PROC_SLOTS:
				return EAGAIN;
			case ERROR_DRIVE_LOCKED:
				return EACCES;
			case ERROR_BROKEN_PIPE:
				return EPIPE;
			case ERROR_DISK_FULL:
				return ENOSPC;
			case ERROR_INVALID_TARGET_HANDLE:
				return EBADF;
			case ERROR_INVALID_LEVEL:
				return EINVAL;
			case ERROR_WAIT_NO_CHILDREN:
			case ERROR_CHILD_NOT_COMPLETE:
				return ECHILD;
			case ERROR_DIRECT_ACCESS_HANDLE:
				return EBADF;
			case ERROR_NEGATIVE_SEEK:
				return EINVAL;
			case ERROR_SEEK_ON_DEVICE:
				return EACCES;
			case ERROR_DIR_NOT_EMPTY:
				return ENOTEMPTY;
			case ERROR_NOT_LOCKED:
				return EACCES;
			case ERROR_BAD_PATHNAME:
				return ENOENT;
			case ERROR_MAX_THRDS_REACHED:
				return EAGAIN;
			case ERROR_LOCK_FAILED:
				return EACCES;
			case ERROR_ALREADY_EXISTS:
				return EEXIST;
			case ERROR_FILENAME_EXCED_RANGE:
				return ENOENT;
			case ERROR_NESTING_NOT_ALLOWED:
				return EAGAIN;
			case ERROR_NOT_ENOUGH_QUOTA:
				return ENOMEM;
			default:
				break;
			}
			if (IsInClosedInterval<ErrorCode>(err, ERROR_WRITE_PROTECT,
				ERROR_SHARING_BUFFER_EXCEEDED))
				return EACCES;
			if (IsInClosedInterval<ErrorCode>(err, ERROR_INVALID_STARTING_CODESEG,
				ERROR_INFLOOP_IN_RELOC_CHAIN))
				return ENOEXEC;
			return EINVAL;
		}

		namespace
		{

			class Win32ErrorCategory : public std::error_category
			{
			public:
				PDefH(const char*, name, ) const lnothrow override
					ImplRet("Win32Error")
					PDefH(std::string, message, int ev) const override
					// NOTE: For Win32 a %::DWORD can be mapped one-to-one for 32-bit %int.
					ImplRet("Error " + std::to_string(ev) + ": "
						+ Win32Exception::FormatMessage(ErrorCode(ev)))
			};


			//@{
			template<typename _func>
			auto
				FetchFileInfo(_func f, UniqueHandle::pointer h)
				-> decltype(f(std::declval<::BY_HANDLE_FILE_INFORMATION&>()))
			{
				::BY_HANDLE_FILE_INFORMATION info;

				LCL_CallF_Win32(GetFileInformationByHandle, h, &info);
				return f(info);
			}

			template<typename _func, typename... _tParams>
			auto
				MakeFileToDo(_func f, _tParams&&... args)
				-> decltype(f(UniqueHandle::pointer()))
			{
				if (const auto h = MakeFile(lforward(args)...))
					return f(h.get());
				LCL_Raise_Win32E("CreateFileW", lfsig);
			}

			lconstfn
				PDefH(FileAttributesAndFlags, FollowToAttr, bool follow_reparse_point) lnothrow
				ImplRet(follow_reparse_point ? FileAttributesAndFlags::NormalWithDirectory
					: FileAttributesAndFlags::NormalAll)
				//@}


				lconstexpr const auto FSCTL_GET_REPARSE_POINT(0x000900A8UL);


			//@{
			// TODO: Extract to %YCLib.NativeAPI?
			lconstfn PDefH(unsigned long, High32, std::uint64_t val) lnothrow
				ImplRet(static_cast<unsigned long>(val >> 32UL))

				lconstfn PDefH(unsigned long, Low32, std::uint64_t val) lnothrow
				ImplRet(static_cast<unsigned long>(val))

				template<typename _func>
			auto
				DoWithDefaultOverlapped(_func f, std::uint64_t off)
				-> decltype(f(std::declval<::OVERLAPPED&>()))
			{
				::OVERLAPPED overlapped{ 0, 0,{ Low32(off), High32(off) },{} };

				return f(overlapped);
			}
			//@}


			//@{
			enum class SystemPaths
			{
				System,
				Windows
			};

			wstring
				FetchFixedSystemPath(SystemPaths e, size_t s)
			{
				// XXX: Depends right behavior on external API.
				const auto p_buf(make_unique_default_init<wchar_t[]>(s));
				const auto str(p_buf.get());

				switch (e)
				{
				case SystemPaths::System:
					LCL_CallF_Win32(GetSystemDirectoryW, str, unsigned(s));
					break;
				case SystemPaths::Windows:
					LCL_CallF_Win32(GetSystemWindowsDirectoryW, str, unsigned(s));
					break;
				}
				return leo::rtrim(wstring(str), L'\\') + L'\\';
			}
			//@}

		} // unnamed namespace;


		Win32Exception::Win32Exception(ErrorCode ec, string_view msg, RecordLevel lv)
			: Exception(int(ec), GetErrorCategory(), msg, lv)
		{
			LAssert(ec != 0, "No error should be thrown.");
		}
		Win32Exception::Win32Exception(ErrorCode ec, string_view msg, const char* fn,
			RecordLevel lv)
			: Win32Exception(ec,string(msg) + " @ " + Nonnull(fn), lv)
		{}
		ImplDeDtor(Win32Exception)

			const std::error_category&
			Win32Exception::GetErrorCategory()
		{
			static const Win32ErrorCategory ecat{};

			return ecat;
		}

		std::string
			Win32Exception::FormatMessage(ErrorCode ec) lnothrow
		{
			return TryInvoke([=] {
				try
				{
					wchar_t* buf{};

					LCL_CallF_Win32(FormatMessageW, FORMAT_MESSAGE_ALLOCATE_BUFFER
						| FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM,
						{}, ec, MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
						reinterpret_cast<wchar_t*>(&buf), 1, {});

					const auto p(unique_raw(buf, LocalDelete()));

					return WCSToMBCS(buf, unsigned(CP_UTF8));
				}
				CatchExpr(..., TraceDe(Warning, "FormatMessage failed."), throw)
			});
		}

		ModuleProc*
			LoadProc(::HMODULE h_module, const char* proc)
		{
			return reinterpret_cast<ModuleProc*>(LCL_CallF_Win32(GetProcAddress, h_module, proc));//cl bug
		}

		wstring
			FetchModuleFileName(::HMODULE h_module, RecordLevel lv)
		{
			// TODO: Avoid retry for NT 6 %::GetModuleFileNameW?
			return leo::retry_for_vector<wstring>(MAX_PATH,
				[=](wstring& res, size_t s) -> bool {
				const auto r(size_t(::GetModuleFileNameW(h_module, &res[0],
					static_cast<unsigned long>(s))));
				const auto err(::GetLastError());

				if (err != ERROR_SUCCESS && err != ERROR_INSUFFICIENT_BUFFER)
					throw Win32Exception(err, "GetModuleFileNameW", lv);
				if (r < s)
				{
					res.resize(r);
					return {};
				}
				return true;
			});
		}

		void
			LocalDelete::operator()(pointer h) const lnothrow
		{
			// FIXME: For some platforms, no %::LocalFree available. See https://msdn.microsoft.com/zh-cn/library/windows/desktop/ms679351(v=vs.85).aspx.
			// NOTE: %::LocalFree ignores null handle value.
			if (LB_UNLIKELY(::LocalFree(h)))
				TraceDe(Warning, "Error %lu: failed calling LocalFree @ %s.",
					::GetLastError(), lfsig);
		}

		wstring
			FetchWindowsPath(size_t s) {
			return FetchFixedSystemPath(SystemPaths::Windows, s);
		}


		NodeCategory
			TryCategorizeNodeAttributes(UniqueHandle::pointer h)
		{
			return FetchFileInfo([&](::BY_HANDLE_FILE_INFORMATION& info)
				-> NodeCategory {
				return CategorizeNode(FileAttributes(info.dwFileAttributes));
			}, h);
		}

		NodeCategory
			TryCategorizeNodeDevice(UniqueHandle::pointer h)
		{
			NodeCategory res;

			switch (::GetFileType(h))
			{
			case FILE_TYPE_CHAR:
				res = NodeCategory::Character;
				break;
			case FILE_TYPE_PIPE:
				res = NodeCategory::FIFO;
				break;
			case FILE_TYPE_UNKNOWN:
			{
				const auto err(::GetLastError());

				if (err != NO_ERROR)
					throw Win32Exception(err, "GetFileType", Err);
			}
			default:
				res = NodeCategory::Unknown;
			}
			return res;
		}

		NodeCategory
			CategorizeNode(FileAttributes attr, unsigned long reparse_tag) lnothrow
		{
			auto res(NodeCategory::Empty);

			if (IsDirectory(attr))
				res |= NodeCategory::Directory;
			if (attr & ReparsePoint)
			{
				switch (reparse_tag)
				{
				case IO_REPARSE_TAG_SYMLINK:
					res |= NodeCategory::SymbolicLink;
					break;
				case IO_REPARSE_TAG_MOUNT_POINT:
					res |= NodeCategory::MountPoint;
				default:
					;
				}
			}
			return res;
		}
		NodeCategory
			CategorizeNode(UniqueHandle::pointer h) lnothrow
		{
			TryRet(TryCategorizeNodeAttributes(h) | TryCategorizeNodeDevice(h))
				CatchIgnore(...)
				return NodeCategory::Invalid;
		}


		UniqueHandle
			MakeFile(const wchar_t* path, FileAccessRights desired_access,
				FileShareMode shared_mode, CreationDisposition creation_disposition,
				FileAttributesAndFlags attributes_and_flags) lnothrowv
		{
			using leo::underlying;
			const auto h(::CreateFileW(Nonnull(path), underlying(
				desired_access), underlying(shared_mode), {}, underlying(
					creation_disposition), underlying(attributes_and_flags), {}));

			return UniqueHandle(h != INVALID_HANDLE_VALUE ? h
				: UniqueHandle::pointer());
		}

		bool
			CheckWine()
		{
			try
			{
				RegistryKey k1(HKEY_CURRENT_USER, L"Software\\Wine");
				RegistryKey k2(HKEY_LOCAL_MACHINE, L"Software\\Wine");

				lunused(k1),
					lunused(k2);
				return true;
			}
			CatchIgnore(Win32Exception&)
				return {};
		}


		void
			DirectoryFindData::Deleter::operator()(pointer p) const lnothrowv
		{
			FilterExceptions(std::bind(LCL_WrapCall_Win32(FindClose, p), lfsig),
				"directory find data deleter");
		}


		DirectoryFindData::DirectoryFindData(wstring name)
			: dir_name(std::move(name)), find_data()
		{
			if (leo::rtrim(dir_name, L"/\\").empty())
				dir_name = L'.';
			LAssert(platform::EndsWithNonSeperator(dir_name),
				"Invalid argument found.");

			const auto attr(FileAttributes(::GetFileAttributesW(dir_name.c_str())));

			if (attr != Invalid)
			{
				if (attr & Directory)
					dir_name += L"\\*";
				else
					leo::throw_error(ENOTDIR, lfsig);
			}
			else
				LCL_Raise_Win32E("GetFileAttributesW", lfsig);
		}

		NodeCategory
			DirectoryFindData::GetNodeCategory() const lnothrow
		{
			if (p_node && find_data.cFileName[0] != wchar_t())
			{
				auto res(CategorizeNode(find_data));
				wstring_view name(GetDirName());

				name.remove_suffix(1);
				TryInvoke([&] {
					auto gd(leo::unique_guard([&]() lnothrow{
						res |= NodeCategory::Invalid;
					}));

					// NOTE: Only existed and accessible files are considered.
					// FIXME: Blocked. TOCTTOU access.
					if (const auto h = MakeFile((wstring(name) + GetEntryName()).c_str(),
						FileSpecificAccessRights::ReadAttributes,
						FileAttributesAndFlags::NormalWithDirectory))
						res |= TryCategorizeNodeAttributes(h.get())
						| TryCategorizeNodeDevice(h.get());
					leo::dismiss(gd);
				});
				return res;
			}
			return NodeCategory::Empty;
		}

		bool
			DirectoryFindData::Read()
		{
			const auto chk_err([this](const char* fn, ErrorCode ec) LB_NONNULL(1) {
				const auto err(::GetLastError());

				if (err != ec)
					throw Win32Exception(err, fn, Err);
			});

			if (!p_node)
			{
				const auto h_node(::FindFirstFileW(GetDirName().c_str(), &find_data));

				if (h_node != INVALID_HANDLE_VALUE)
					p_node.reset(h_node);
				else
					chk_err("FindFirstFileW", ERROR_FILE_NOT_FOUND);
			}
			else if (!::FindNextFileW(p_node.get(), &find_data))
			{
				chk_err("FindNextFileW", ERROR_NO_MORE_FILES);
				p_node.reset();
			}
			if (p_node)
				return find_data.cFileName[0] != wchar_t();
			find_data.cFileName[0] = wchar_t();
			return {};
		}

		void
			DirectoryFindData::Rewind() lnothrow
		{
			p_node.reset();
		}
	}
#endif
}