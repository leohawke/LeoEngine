/*!	\file MinGW32.h
\ingroup Framework
\ingroup Win32
\brief Framework MinGW32 平台公共扩展。
*/

#ifndef FrameWork_Win32_Mingw32_h
#define FrameWork_Win32_Mingw32_h 1

#include <LFramework/LCLib/Host.h>
#include <LFramework/LCLib/NativeAPI.h>
#if !LFL_Win32
#	error "This file is only for Win32."
#endif
#include <LFramework/LCLib/Debug.h>
#include <LBase/enum.hpp>
#include <LFramework/LCLib/FReference.h> //for unique_ptr todo FileIO.h
#include <chrono>

namespace platform_ex {

	namespace Windows {
		using ErrorCode = unsigned long;

		/*!
		\brief 转换 Win32 错误为 errno 。
		\return 当对应不存在时 EINVAL ，否则参数对应的 errno 。
		*/
		LF_API LB_STATELESS int
			ConvertToErrno(ErrorCode) lnothrow;

		/*!
		\brief 取转换为 errno 的 Win32 错误。
		*/
		inline PDefH(int, GetErrnoFromWin32, ) lnothrow
			ImplRet(ConvertToErrno(::GetLastError()))

		/*!
		\ingroup exceptions
		\brief Win32 错误引起的宿主异常。
		*/
		class LB_API Win32Exception : public Exception
		{
		public:
			/*!
			\pre 错误码不等于 0 。
			\warning 初始化参数时可能会改变 ::GetLastError() 的结果。
			*/
			//@{
			Win32Exception(ErrorCode, string_view = "Win32 exception",
				leo::RecordLevel = leo::Emergent);
			/*!
			\pre 第三参数非空。
			\note 第三参数表示函数名，可以使用 \c __func__ 。
			*/
			LB_NONNULL(4)
				Win32Exception(ErrorCode, string_view, const char*,
					leo::RecordLevel = leo::Emergent);
			//@}
			DefDeCopyCtor(Win32Exception)
				/*!
				\brief 虚析构：类定义外默认实现。
				*/
				~Win32Exception() override;

			DefGetter(const lnothrow, ErrorCode, ErrorCode, ErrorCode(code().value()))
				DefGetter(const lnothrow, std::string, Message,
					FormatMessage(GetErrorCode()))

				explicit DefCvt(const lnothrow, ErrorCode, GetErrorCode())

				/*!
				\brief 取错误类别。
				\return std::error_category 派生类的 const 引用。
				*/
				static const std::error_category&
				GetErrorCategory();

			/*!
			\brief 格式化错误消息字符串。
			\return 若发生异常则结果为空，否则为区域固定为 en-US 的系统消息字符串。
			*/
			static std::string
				FormatMessage(ErrorCode) lnothrow;
			//@}
		};


		//@{
		//! \brief 按 ::GetLastError 的结果和指定参数抛出 Windows::Win32Exception 对象。
#	define LCL_Raise_Win32E(...) \
	{ \
		const auto err(::GetLastError()); \
	\
		throw platform_ex::Windows::Win32Exception(err, __VA_ARGS__); \
	}

		//! \brief 按表达式求值和指定参数抛出 Windows::Win32Exception 对象。
#	define LCL_RaiseZ_Win32E(_expr, ...) \
	{ \
		const auto err(Windows::ErrorCode(_expr)); \
	\
		if(err != ERROR_SUCCESS) \
			throw platform_ex::Windows::Win32Exception(err, __VA_ARGS__); \
	}
		//@}

		/*!
		\brief 跟踪 ::GetLastError 取得的调用状态结果。
		*/
#	define LCL_Trace_Win32E(_lv, _fn, _msg) \
	TraceDe(_lv, "Error %lu: failed calling " #_fn " @ %s.", \
		::GetLastError(), _msg)

		/*!
		\brief 调用 Win32 API 或其它可用 ::GetLastError 取得调用状态的例程。
		\note 调用时直接使用实际参数，可指定非标识符的表达式，不保证是全局名称。
		*/
		//@{
		/*!
		\note 若失败抛出 Windows::Win32Exception 对象。
		*/
		//@{
#	define LCL_WrapCall_Win32(_fn, ...) \
	[&](const char* msg) LB_NONNULL(1){ \
		const auto res(_fn(__VA_ARGS__)); \
	\
		if LB_UNLIKELY(!res) \
			LCL_Raise_Win32E(#_fn, msg); \
		return res; \
	}

#	define LCL_Call_Win32(_fn, _msg, ...) \
	LCL_WrapCall_Win32(_fn, __VA_ARGS__)(_msg)

#	define LCL_CallF_Win32(_fn, ...) LCL_Call_Win32(_fn, lfsig, __VA_ARGS__)
		//@}

		/*!
		\note 若失败跟踪 ::GetLastError 的结果。
		\note 格式转换说明符置于最前以避免宏参数影响结果。
		\sa LCL_Trace_Win32E
		*/
		//@{
#	define LCL_WrapCallWin32_Trace(_fn, ...) \
	[&](const char* msg) LB_NONNULL(1){ \
		const auto res(_fn(__VA_ARGS__)); \
	\
		if LB_UNLIKELY(!res) \
			LCL_Trace_Win32E(platform::Descriptions::Warning, _fn, msg); \
		return res; \
	}

#	define LFL_CallWin32_Trace(_fn, _msg, ...) \
	LCL_WrapCallWin32_Trace(_fn, __VA_ARGS__)(_msg)

#	define LFL_CallWin32F_Trace(_fn, ...) \
	LFL_CallWin32_Trace(_fn, lfsig, __VA_ARGS__)
		//@}
		//@}

		//! \since for Load D3D12
		//@{
		//! \brief 加载过程地址得到的过程类型。
		using ModuleProc = std::remove_reference_t<decltype(*::GetProcAddress(::HMODULE(), {})) > ;

		/*!
		\brief 从模块加载指定过程的指针。
		\pre 参数非空。
		*/
		//@{
		LB_API LB_ATTR_returns_nonnull LB_NONNULL(2) ModuleProc*
			LoadProc(::HMODULE, const char*);
		template<typename _func>
		inline LB_NONNULL(2) _func&
			LoadProc(::HMODULE h_module, const char* proc)
		{
			return  *platform::FwdIter(reinterpret_cast<_func*>(LoadProc(h_module, proc)));
			//return platform::Deref(reinterpret_cast<_func*>(LoadProc(h_module, proc))); cl bug
		}
		template<typename _func>
		LB_NONNULL(1, 2) _func&
			LoadProc(const wchar_t* module, const char* proc)
		{
			return LoadProc<_func>(LCL_CallF_Win32(GetModuleHandleW, module), proc);
		}

#define LCL_Impl_W32Call_Fn(_fn) W32_##_fn##_t
#define LCL_Impl_W32Call_FnCall(_fn) W32_##_fn##_call

		/*!
		\brief 声明调用 WinAPI 的例程。
		\note 为避免歧义，应在非全局命名空间中使用；注意类作用域名称查找顺序会引起歧义。
		\note 当没有合式调用时从指定模块加载。
		*/
#define LCL_DeclW32Call(_fn, _module, _tRet, ...) \
	LCL_DeclCheck_t(_fn, _fn) \
	using LCL_Impl_W32Call_Fn(_fn) = _tRet __stdcall(__VA_ARGS__); \
	\
	template<typename... _tParams> \
	auto \
	LCL_Impl_W32Call_FnCall(_fn)(_tParams&&... args) \
		-> LCL_CheckDecl_t(_fn)<_tParams...> \
	{ \
		return _fn(lforward(args)...); \
	} \
	template<typename... _tParams> \
	leo::enable_fallback_t<LCL_CheckDecl_t(_fn), \
		LCL_Impl_W32Call_Fn(_fn), _tParams...> \
	LCL_Impl_W32Call_FnCall(_fn)(_tParams&&... args) \
	{ \
		return platform_ex::Windows::LoadProc<LCL_Impl_W32Call_Fn(_fn)>( \
			L###_module, #_fn)(lforward(args)...); \
	} \
	\
	template<typename... _tParams> \
	auto \
	_fn(_tParams&&... args) \
		-> decltype(LCL_Impl_W32Call_FnCall(_fn)(lforward(args)...)) \
	{ \
		return LCL_Impl_W32Call_FnCall(_fn)(lforward(args)...); \
	}
		//@}

		LF_API wstring
			FetchModuleFileName(::HMODULE = {}, leo::RecordLevel = leo::Err);

			/*!
			\brief 局部存储删除器。
			*/
		struct LB_API LocalDelete
		{
			using pointer = ::HLOCAL;

			void
				operator()(pointer) const lnothrow;
		};


		//@{
		/*!
		\brief 访问权限。
		\see https://msdn.microsoft.com/en-us/library/windows/desktop/aa374892(v=vs.85).aspx 。
		\see https://msdn.microsoft.com/en-us/library/windows/desktop/aa374896(v=vs.85).aspx 。
		*/
		enum class AccessRights : ::ACCESS_MASK
		{
			None = 0,
			GenericRead = GENERIC_READ,
			GenericWrite = GENERIC_WRITE,
			GenericReadWrite = GenericRead | GenericWrite,
			GenericExecute = GENERIC_EXECUTE,
			GenericAll = GENERIC_ALL,
			MaximumAllowed = MAXIMUM_ALLOWED,
			AccessSystemACL = ACCESS_SYSTEM_SECURITY,
			Delete = DELETE,
			ReadControl = READ_CONTROL,
			Synchronize = SYNCHRONIZE,
			WriteDAC = WRITE_DAC,
			WriteOwner = WRITE_OWNER,
			All = STANDARD_RIGHTS_ALL,
			Execute = STANDARD_RIGHTS_EXECUTE,
			StandardRead = STANDARD_RIGHTS_READ,
			Required = STANDARD_RIGHTS_REQUIRED,
			StandardWrite = STANDARD_RIGHTS_WRITE
		};

		//! \relates AccessRights
		DefBitmaskEnum(AccessRights)

		//! \brief 文件特定的访问权限。
		enum class FileSpecificAccessRights : ::ACCESS_MASK
		{
			AddFile = FILE_ADD_FILE,
			AddSubdirectory = FILE_ADD_SUBDIRECTORY,
			AllAccess = FILE_ALL_ACCESS,
			AppendData = FILE_APPEND_DATA,
			CreatePipeInstance = FILE_CREATE_PIPE_INSTANCE,
			DeleteChild = FILE_DELETE_CHILD,
			Execute = FILE_EXECUTE,
			ListDirectory = FILE_LIST_DIRECTORY,
			ReadAttributes = FILE_READ_ATTRIBUTES,
			ReadData = FILE_READ_DATA,
			ReadEA = FILE_READ_EA,
			Traverse = FILE_TRAVERSE,
			WriteAttributes = FILE_WRITE_ATTRIBUTES,
			WriteData = FILE_WRITE_DATA,
			WriteEA = FILE_WRITE_EA,
			Read = STANDARD_RIGHTS_READ,
			Write = STANDARD_RIGHTS_WRITE
		};

		//! \relates FileSpecificAccessRights
		DefBitmaskEnum(FileSpecificAccessRights)


			//! \brief 文件访问权限。
			using FileAccessRights
			= leo::enum_union<AccessRights, FileSpecificAccessRights>;

		//! \relates FileAccessRights
		DefBitmaskOperations(FileAccessRights,
			leo::wrapped_enum_traits_t<FileAccessRights>)


			//! \brief 文件共享模式。
			enum class FileShareMode : ::ACCESS_MASK
		{
			None = 0,
			Delete = FILE_SHARE_DELETE,
			Read = FILE_SHARE_READ,
			Write = FILE_SHARE_WRITE,
			ReadWrite = Read | Write,
			All = Delete | Read | Write
		};

		//! \relates FileShareMode
		DefBitmaskEnum(FileShareMode)


			//! \brief 文件创建选项。
		enum class CreationDisposition : unsigned long
		{
			CreateAlways = CREATE_ALWAYS,
			CreateNew = CREATE_NEW,
			OpenAlways = OPEN_ALWAYS,
			OpenExisting = OPEN_EXISTING,
			TruncateExisting = TRUNCATE_EXISTING
		};
		//@}


		//@{
		//! \see https://msdn.microsoft.com/en-us/library/gg258117(v=vs.85).aspx 。
		enum FileAttributes : unsigned long
		{
			ReadOnly = FILE_ATTRIBUTE_READONLY,
			Hidden = FILE_ATTRIBUTE_HIDDEN,
			System = FILE_ATTRIBUTE_SYSTEM,
			Directory = FILE_ATTRIBUTE_DIRECTORY,
			Archive = FILE_ATTRIBUTE_ARCHIVE,
			Device = FILE_ATTRIBUTE_DEVICE,
			Normal = FILE_ATTRIBUTE_NORMAL,
			Temporary = FILE_ATTRIBUTE_TEMPORARY,
			SparseFile = FILE_ATTRIBUTE_SPARSE_FILE,
			ReparsePoint = FILE_ATTRIBUTE_REPARSE_POINT,
			Compressed = FILE_ATTRIBUTE_COMPRESSED,
			Offline = FILE_ATTRIBUTE_OFFLINE,
			NotContentIndexed = FILE_ATTRIBUTE_NOT_CONTENT_INDEXED,
			Encrypted = FILE_ATTRIBUTE_ENCRYPTED,
			//	IntegrityStream = FILE_ATTRIBUTE_INTEGRITY_STREAM,
			IntegrityStream = 0x8000,
			Virtual = FILE_ATTRIBUTE_VIRTUAL,
			//	NoScrubData = FILE_ATTRIBUTE_NO_SCRUB_DATA,
			NoScrubData = 0x20000,
			//	EA = FILE_ATTRIBUTE_EA,
			//! \warning 非 MSDN 文档公开。
			EA = 0x40000,
			Invalid = INVALID_FILE_ATTRIBUTES
		};

		//! \see https://msdn.microsoft.com/en-us/library/aa363858(v=vs.85).aspx 。
		enum FileFlags : unsigned long
		{
			WriteThrough = FILE_FLAG_WRITE_THROUGH,
			Overlapped = FILE_FLAG_OVERLAPPED,
			NoBuffering = FILE_FLAG_NO_BUFFERING,
			RandomAccess = FILE_FLAG_RANDOM_ACCESS,
			SequentialScan = FILE_FLAG_SEQUENTIAL_SCAN,
			DeleteOnClose = FILE_FLAG_DELETE_ON_CLOSE,
			BackupSemantics = FILE_FLAG_BACKUP_SEMANTICS,
			POSIXSemantics = FILE_FLAG_POSIX_SEMANTICS,
			SessionAware = FILE_FLAG_SESSION_AWARE,
			OpenReparsePoint = FILE_FLAG_OPEN_REPARSE_POINT,
			OpenNoRecall = FILE_FLAG_OPEN_NO_RECALL,
			// \see https://msdn.microsoft.com/zh-cn/library/windows/desktop/aa365150(v=vs.85).aspx 。
			FirstPipeInstance = FILE_FLAG_FIRST_PIPE_INSTANCE
		};

		enum SecurityQoS : unsigned long
		{
			Anonymous = SECURITY_ANONYMOUS,
			ContextTracking = SECURITY_CONTEXT_TRACKING,
			Delegation = SECURITY_DELEGATION,
			EffectiveOnly = SECURITY_EFFECTIVE_ONLY,
			Identification = SECURITY_IDENTIFICATION,
			Impersonation = SECURITY_IMPERSONATION,
			Present = SECURITY_SQOS_PRESENT,
			ValidFlags = SECURITY_VALID_SQOS_FLAGS
		};

		enum FileAttributesAndFlags : unsigned long
		{
			NormalWithDirectory = Normal | BackupSemantics,
			NormalAll = NormalWithDirectory | OpenReparsePoint
		};
		//@}


		/*!
		\brief 判断 \c FileAttributes 是否指定目录。
		*/
		lconstfn PDefH(bool, IsDirectory, FileAttributes attr) lnothrow
			ImplRet(attr & Directory)
			/*!
			\brief 判断 \c ::WIN32_FIND_DATAA 指定的节点是否为目录。
			*/
			inline PDefH(bool, IsDirectory, const ::WIN32_FIND_DATAA& d) lnothrow
			ImplRet(IsDirectory(FileAttributes(d.dwFileAttributes)))
			/*!
			\brief 判断 \c ::WIN32_FIND_DATAW 指定的节点是否为目录。
			*/
			inline PDefH(bool, IsDirectory, const ::WIN32_FIND_DATAW& d) lnothrow
			ImplRet(IsDirectory(FileAttributes(d.dwFileAttributes)))

			/*!
			\throw Win32Exception 调用失败。
			*/
			//@{
			/*!
			\brief 按打开的文件句柄归类节点从属的属性类别。
			\return 指定句柄为字符、管道或未知类别。
			*/
			LF_API platform::NodeCategory
			TryCategorizeNodeAttributes(UniqueHandle::pointer);

		/*!
		\brief 按打开的文件句柄归类节点从属的设备类别。
		\return 指定句柄为字符、管道或未知类别。
		*/
		LF_API platform::NodeCategory
			TryCategorizeNodeDevice(UniqueHandle::pointer);
		//@}

		/*!
		\return 指定非目录或不被支持的重解析标签时为 NodeCategory::Empty ，
		否则为对应的目录和重解析标签的组合节点类别。
		\todo 对目录链接和符号链接的重解析标签提供适当实现。
		*/
		//@{
		//! \brief 按 FileAttributes 和重解析标签归类节点类别。
		LF_API platform::NodeCategory
			CategorizeNode(FileAttributes, unsigned long = 0) lnothrow;
		//! \brief 按 \c ::WIN32_FIND_DATAA 归类节点类别。
		inline PDefH(platform::NodeCategory, CategorizeNode,
			const ::WIN32_FIND_DATAA& d) lnothrow
			ImplRet(CategorizeNode(FileAttributes(d.dwFileAttributes), d.dwReserved0))
			/*!
			\brief 按 \c ::WIN32_FIND_DATAW 归类节点类别。
			*/
			inline PDefH(platform::NodeCategory, CategorizeNode,
				const ::WIN32_FIND_DATAW& d) lnothrow
			ImplRet(CategorizeNode(FileAttributes(d.dwFileAttributes), d.dwReserved0))
			//@}
			/*!
			\brief 按打开的文件句柄归类节点类别。
			\return 指定句柄空时为 NodeCategory::Invalid ，
			否则为文件属性和设备类别查询的位或结果。
			\sa TryCategorizeNodeAttributes
			\sa TryCategorizeNodeDevice
			*/
			LF_API platform::NodeCategory
			CategorizeNode(UniqueHandle::pointer) lnothrow;


		/*!
		\brief 创建或打开独占的文件或设备。
		\pre 间接断言：路径参数非空。
		\note 调用 \c ::CreateFileW 实现。
		*/
		//@{
		LF_API LB_NONNULL(1) UniqueHandle
			MakeFile(const wchar_t*, FileAccessRights = AccessRights::None,
				FileShareMode = FileShareMode::All, CreationDisposition
				= CreationDisposition::OpenExisting,
				FileAttributesAndFlags = FileAttributesAndFlags::NormalAll) lnothrowv;
		//! \since build 660
		inline LB_NONNULL(1) PDefH(UniqueHandle, MakeFile, const wchar_t* path,
			FileAccessRights desired_access, FileShareMode shared_mode,
			FileAttributesAndFlags attributes_and_flags) lnothrowv
			ImplRet(MakeFile(path, desired_access, shared_mode,
				CreationDisposition::OpenExisting, attributes_and_flags))
			inline LB_NONNULL(1) PDefH(UniqueHandle, MakeFile, const wchar_t* path,
				FileAccessRights desired_access, CreationDisposition creation_disposition,
				FileAttributesAndFlags attributes_and_flags
				= FileAttributesAndFlags::NormalAll) lnothrowv
			ImplRet(MakeFile(path, desired_access, FileShareMode::All,
				creation_disposition, attributes_and_flags))
			inline LB_NONNULL(1) PDefH(UniqueHandle, MakeFile, const wchar_t* path,
				FileAccessRights desired_access,
				FileAttributesAndFlags attributes_and_flags) lnothrowv
			ImplRet(MakeFile(path, desired_access, FileShareMode::All,
				CreationDisposition::OpenExisting, attributes_and_flags))
			//@{
			inline LB_NONNULL(1) PDefH(UniqueHandle, MakeFile, const wchar_t* path,
				FileShareMode shared_mode, CreationDisposition creation_disposition
				= CreationDisposition::OpenExisting, FileAttributesAndFlags
				attributes_and_flags = FileAttributesAndFlags::NormalAll) lnothrowv
			ImplRet(MakeFile(path, AccessRights::None, shared_mode,
				creation_disposition, attributes_and_flags))
			inline LB_NONNULL(1) PDefH(UniqueHandle, MakeFile, const wchar_t* path,
				CreationDisposition creation_disposition, FileAttributesAndFlags
				attributes_and_flags = FileAttributesAndFlags::NormalAll) lnothrowv
			ImplRet(MakeFile(path, AccessRights::None, FileShareMode::All,
				creation_disposition, attributes_and_flags))
			inline LB_NONNULL(1) PDefH(UniqueHandle, MakeFile, const wchar_t* path,
				FileShareMode shared_mode, FileAttributesAndFlags
				attributes_and_flags = FileAttributesAndFlags::NormalAll) lnothrowv
			ImplRet(MakeFile(path, AccessRights::None, shared_mode,
				CreationDisposition::OpenExisting, attributes_and_flags))
			inline LB_NONNULL(1) PDefH(UniqueHandle, MakeFile, const wchar_t* path,
				FileAttributesAndFlags attributes_and_flags) lnothrowv
			ImplRet(MakeFile(path, AccessRights::None, FileShareMode::All,
				CreationDisposition::OpenExisting, attributes_and_flags))
			//@}
			//@}


			/*!
			\brief 判断是否在 Wine 环境下运行。
			\note 检查 \c HKEY_CURRENT_USER 和 \c HKEY_LOCAL_MACHINE
			下的 <tt>Software\Wine</tt> 键实现。
			*/
			LF_API bool
			CheckWine();


		/*!
		\brief 文件系统目录查找状态。
		\warning 非虚析构。
		*/
		class LF_API DirectoryFindData : private leo::noncopyable
		{
		private:
			class LF_API Deleter
			{
			public:
				using pointer = ::HANDLE;

				void
					operator()(pointer) const lnothrowv;
			};

			/*!
			\brief 查找起始的目录名称。
			\invariant <tt>dir_name.length() > 1
			&& leo::ends_with(dir_name, L"\\*")</tt> 。
			\sa GetDirName
			*/
			wstring dir_name;
			//! \brief Win32 查找数据。
			::WIN32_FIND_DATAW find_data;
			/*!
			\brief 查找节点。
			*/
			unique_ptr_from<Deleter> p_node{};

		public:
			/*!
			\brief 构造：使用指定的目录路径。
			\pre 间接断言：路径参数的数据指针非空。
			\throw std::system_error 指定的路径不是目录。
			\throw Win32Exception 路径属性查询失败。

			打开 UTF-16 路径指定的目录。
			目录路径无视结尾的斜杠和反斜杠。 去除结尾斜杠和反斜杠后若为空则视为当前路径。
			*/
			//@{
			DirectoryFindData(wstring_view sv)
				: DirectoryFindData(wstring(sv))
			{}
			DirectoryFindData(u16string_view sv)
				: DirectoryFindData(wstring(sv.cbegin(), sv.cend()))
			{}
			DirectoryFindData(wstring);
			//@}
			//! \brief 析构：若查找节点句柄非空则关闭查找状态。
			DefDeDtor(DirectoryFindData)

				DefBoolNeg(explicit, p_node.get())

				DefGetter(const lnothrow, unsigned long, Attributes,
					find_data.dwFileAttributes)
				/*!
				\brief 返回当前查找项目名。
				\return 当前查找项目名。
				*/
				LB_PURE DefGetter(const lnothrow,
					const wchar_t*, EntryName, find_data.cFileName)
				DefGetter(const lnothrow, const ::WIN32_FIND_DATAW&, FindData, find_data)
				DefGetter(const lnothrow, const wstring&, DirName, (LAssert(
					dir_name.length() > 1 && leo::ends_with(dir_name, L"\\*"),
					"Invalid directory name found."), dir_name))
				/*!
				\brief 取子节点的类型。
				\return 当前节点无效或查找的项目名称为空时为 platform::NodeCategory::Empty ，
				否则为处理文件信息得到的值。
				\sa CategorizeNode

				处理文件信息时，首先调用 CategorizeNode 对查找数据归类；
				然后打开名称指定的文件进一步调用 CategorizeNode 对打开的文件判断归类。
				*/
				platform::NodeCategory
				GetNodeCategory() const lnothrow;

			/*!
			\brief 读取：迭代当前查找状态。
			\throw Win32Exception 读取失败。
			\return 若迭代结束后节点且文件名非空。

			若查找节点句柄非空则迭代当前查找状态查找下一个文件系统项。
			否则查找节点句柄为空或迭代失败则关闭查找状态并置查找节点句柄空。
			最终查找节点非空时保存记录当前查找的项目状态。
			*/
			bool
				Read();

			//! \brief 复位查找状态：若查找节点句柄非空则关闭查找状态并置查找节点句柄空。
			void
				Rewind() lnothrow;
		};

		/*!
		\brief 重解析点数据。
		\note 避免直接使用指针转换成显式不同的类型时引起未定义行为。
		\warning 非虚析构。
		*/
		class LF_API ReparsePointData
		{
		public:
			struct Data;

		private:
			leo::aligned_storage_t<MAXIMUM_REPARSE_DATA_BUFFER_SIZE,
				lalignof(void*)> target_buffer;
			//! \invariant <tt>&pun.get() == &target_buffer</tt>
			leo::pun_ref<Data> pun;

		public:
			ReparsePointData();
			/*!
			\brief 析构：类定义外默认实现。
			\note 允许 Data 作为不完整类型使用。
			*/
			~ReparsePointData();

			DefGetter(const lnothrow, Data&, , pun.get())
		};


		/*!
		\brief 读取重解析点内容。
		\pre 断言：参数非空。
		\exception Win32Exception 打开文件失败。
		\throw std::invalid_argument 打开的文件不是重解析点。
		\throw std::system_error 重解析点检查失败。
		\li std::errc::not_supported 重解析点标签不被支持。
		*/
		//@{
		LF_API LB_NONNULL(1) wstring
			ResolveReparsePoint(const wchar_t*);
		LF_API LB_NONNULL(1) wstring_view
			ResolveReparsePoint(const wchar_t*, ReparsePointData::Data&);
		//@}

		/*!
		\brief 展开字符串中的环境变量。
		\pre 间接断言：参数非空。
		\throw Win32Exception 调用失败。
		*/
		LF_API LB_NONNULL(1) wstring
			ExpandEnvironmentStrings(const wchar_t*);


		/*!
		\see https://msdn.microsoft.com/zh-cn/library/windows/desktop/aa363788(v=vs.85).aspx 。
		*/
		//@{
		//! \brief 文件标识。
		using FileID = std::uint64_t;
		//! \brief 卷序列号。
		using VolumeID = std::uint32_t;
		//@}

		//! \throw Win32Exception 访问文件或查询文件元数据失败。
		//@{
		//@{
		//! \brief 查询文件链接数。
		//@{
		LF_API size_t
			QueryFileLinks(UniqueHandle::pointer);
		/*!
		\pre 间接断言：路径参数非空。
		\note 最后参数表示跟踪重解析点。
		*/
		LF_API LB_NONNULL(1) size_t
			QueryFileLinks(const wchar_t*, bool = {});
		//@}

		/*!
		\brief 查询文件标识。
		\return 卷标识和卷上文件的标识的二元组。
		\bug ReFS 上不保证唯一。
		\see https://msdn.microsoft.com/zh-cn/library/windows/desktop/aa363788(v=vs.85).aspx 。
		*/
		//@{
		LF_API pair<VolumeID, FileID>
			QueryFileNodeID(UniqueHandle::pointer);
		/*!
		\pre 间接断言：路径参数非空。
		\note 最后参数表示跟踪重解析点。
		*/
		LF_API LB_NONNULL(1) pair<VolumeID, FileID>
			QueryFileNodeID(const wchar_t*, bool = {});
		//@}
		//@}

		/*!
		\brief 查询文件大小。
		\throw std::invalid_argument 查询文件得到的大小小于 0 。
		*/
		//@{
		LF_API std::uint64_t
			QueryFileSize(UniqueHandle::pointer);
		//! \pre 间接断言：路径参数非空。
		LF_API LB_NONNULL(1) std::uint64_t
			QueryFileSize(const wchar_t*);
		//@}

		/*
		\note 后三个参数可选，指针为空时忽略。
		\note 最高精度取决于文件系统。
		*/
		//@{
		//! \brief 查询文件的创建、访问和/或修改时间。
		//@{
		/*!
		\pre 文件句柄不为 \c INVALID_HANDLE_VALUE ，
		且具有 AccessRights::GenericRead 权限。
		*/
		LF_API void
			QueryFileTime(UniqueHandle::pointer, ::FILETIME* = {}, ::FILETIME* = {},
				::FILETIME* = {});
		/*!
		\pre 间接断言：路径参数非空。
		\note 即使可选参数都为空指针时仍访问文件。最后参数表示跟踪重解析点。
		*/
		LF_API LB_NONNULL(1) void
			QueryFileTime(const wchar_t*, ::FILETIME* = {}, ::FILETIME* = {},
				::FILETIME* = {}, bool = {});
		//@}

		/*!
		\brief 设置文件的创建、访问和/或修改时间。
		*/
		//@{
		/*!
		\pre 文件句柄不为 \c INVALID_HANDLE_VALUE ，
		且具有 FileSpecificAccessRights::WriteAttributes 权限。
		*/
		LF_API void
			SetFileTime(UniqueHandle::pointer, ::FILETIME* = {}, ::FILETIME* = {},
				::FILETIME* = {});
		/*!
		\pre 间接断言：路径参数非空。
		\note 即使可选参数都为空指针时仍访问文件。最后参数表示跟踪重解析点。
		*/
		LF_API LB_NONNULL(1) void
			SetFileTime(const wchar_t*, ::FILETIME* = {}, ::FILETIME* = {},
				::FILETIME* = {}, bool = {});
		//@}
		//@}
		//@}

		/*!
		\throw std::system_error 调用失败。
		\li std::errc::not_supported 输入的时间表示不被实现支持。
		*/
		//@{
		/*!
		\brief 转换文件时间为以 POSIX 历元起始度量的时间间隔。
		*/
		LF_API std::chrono::nanoseconds
			ConvertTime(const ::FILETIME&);
		/*!
		\brief 转换以 POSIX 历元起始度量的时间间隔为文件时间。
		*/
		LF_API::FILETIME
			ConvertTime(std::chrono::nanoseconds);
		//@}


		/*!
		\pre 文件句柄不为 \c INVALID_HANDLE_VALUE ，
		且具有 AccessRights::GenericRead 或 AccessRights::GenericWrite 权限。
		*/
		//@{
		/*!
		\brief 锁定文件。
		\note 对内存映射文件为协同锁，其它文件为强制锁。
		\note 第二和第三参数指定文件锁定范围的起始偏移量和大小。
		\note 最后两个参数分别表示是否为独占锁和是否立刻返回。
		*/
		//@{
		//! \throw Win32Exception 锁定失败。
		void
			LockFile(UniqueHandle::pointer, std::uint64_t = 0,
				std::uint64_t = std::uint64_t(-1), bool = true, bool = {});

		bool
			TryLockFile(UniqueHandle::pointer, std::uint64_t = 0,
				std::uint64_t = std::uint64_t(-1), bool = true, bool = true) lnothrow;
		//@}

		/*!
		\brief 解锁文件。
		\pre 文件已被锁定。
		*/
		bool
			UnlockFile(UniqueHandle::pointer, std::uint64_t = 0,
				std::uint64_t = std::uint64_t(-1)) lnothrow;
		//@}

		/*!
		\brief 取系统目录路径。
		*/
		wstring
			FetchWindowsPath(size_t = MAX_PATH);

		struct LF_API GlobalDelete
		{
			using pointer = ::HGLOBAL;

			void
				operator()(pointer) const lnothrow;
		};

		class LF_API GlobalLocked
		{
		private:
			//! \invariant <tt>bool(p_locked)</tt> 。
			void* p_locked;

		public:
			/*!
			\brief 构造：锁定存储。
			\throw Win32Exception ::GlobalLock 调用失败。
			*/
			//@{
			GlobalLocked(::HGLOBAL);
			template<typename _tPointer>
			GlobalLocked(const _tPointer& p)
				: GlobalLocked(p.get())
			{}
			//@}
			~GlobalLocked();

			template<typename _type = void>
			observer_ptr<_type>
				GetPtr() const lnothrow
			{
				return make_observer(static_cast<_type*>(p_locked));
			}
		};
	}
}

#endif

