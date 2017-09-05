/*!	\file MinGW32.h
\ingroup Framework
\ingroup Win32
\brief Framework MinGW32 ƽ̨������չ��
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
		\brief ת�� Win32 ����Ϊ errno ��
		\return ����Ӧ������ʱ EINVAL �����������Ӧ�� errno ��
		*/
		LF_API LB_STATELESS int
			ConvertToErrno(ErrorCode) lnothrow;

		/*!
		\brief ȡת��Ϊ errno �� Win32 ����
		*/
		inline PDefH(int, GetErrnoFromWin32, ) lnothrow
			ImplRet(ConvertToErrno(::GetLastError()))

		/*!
		\ingroup exceptions
		\brief Win32 ��������������쳣��
		*/
		class LB_API Win32Exception : public Exception
		{
		public:
			/*!
			\pre �����벻���� 0 ��
			\warning ��ʼ������ʱ���ܻ�ı� ::GetLastError() �Ľ����
			*/
			//@{
			Win32Exception(ErrorCode, string_view = "Win32 exception",
				leo::RecordLevel = leo::Emergent);
			/*!
			\pre ���������ǿա�
			\note ����������ʾ������������ʹ�� \c __func__ ��
			*/
			LB_NONNULL(4)
				Win32Exception(ErrorCode, string_view, const char*,
					leo::RecordLevel = leo::Emergent);
			//@}
			DefDeCopyCtor(Win32Exception)
				/*!
				\brief ���������ඨ����Ĭ��ʵ�֡�
				*/
				~Win32Exception() override;

			DefGetter(const lnothrow, ErrorCode, ErrorCode, ErrorCode(code().value()))
				DefGetter(const lnothrow, std::string, Message,
					FormatMessage(GetErrorCode()))

				explicit DefCvt(const lnothrow, ErrorCode, GetErrorCode())

				/*!
				\brief ȡ�������
				\return std::error_category ������� const ���á�
				*/
				static const std::error_category&
				GetErrorCategory();

			/*!
			\brief ��ʽ��������Ϣ�ַ�����
			\return �������쳣����Ϊ�գ�����Ϊ����̶�Ϊ en-US ��ϵͳ��Ϣ�ַ�����
			*/
			static std::string
				FormatMessage(ErrorCode) lnothrow;
			//@}
		};


		//@{
		//! \brief �� ::GetLastError �Ľ����ָ�������׳� Windows::Win32Exception ����
#	define LCL_Raise_Win32E(...) \
	{ \
		const auto err(::GetLastError()); \
	\
		throw platform_ex::Windows::Win32Exception(err, __VA_ARGS__); \
	}

		//! \brief ������ʽ��ֵ��ָ�������׳� Windows::Win32Exception ����
#	define LCL_RaiseZ_Win32E(_expr, ...) \
	{ \
		const auto err(Windows::ErrorCode(_expr)); \
	\
		if(err != ERROR_SUCCESS) \
			throw platform_ex::Windows::Win32Exception(err, __VA_ARGS__); \
	}
		//@}

		/*!
		\brief ���� ::GetLastError ȡ�õĵ���״̬�����
		*/
#	define LCL_Trace_Win32E(_lv, _fn, _msg) \
	TraceDe(_lv, "Error %lu: failed calling " #_fn " @ %s.", \
		::GetLastError(), _msg)

		/*!
		\brief ���� Win32 API ���������� ::GetLastError ȡ�õ���״̬�����̡�
		\note ����ʱֱ��ʹ��ʵ�ʲ�������ָ���Ǳ�ʶ���ı���ʽ������֤��ȫ�����ơ�
		*/
		//@{
		/*!
		\note ��ʧ���׳� Windows::Win32Exception ����
		*/
		//@{
#	define LCL_WrapCall_Win32(_fn, ...) \
	[&](const char* msg) LB_NONNULL(1){ \
		const auto res(_fn(__VA_ARGS__)); \
	\
		if(LB_UNLIKELY(!res)) \
			LCL_Raise_Win32E(#_fn, msg); \
		return res; \
	}

#	define LCL_Call_Win32(_fn, _msg, ...) \
	LCL_WrapCall_Win32(_fn, __VA_ARGS__)(_msg)

#	define LCL_CallF_Win32(_fn, ...) LCL_Call_Win32(_fn, lfsig, __VA_ARGS__)
		//@}

		/*!
		\note ��ʧ�ܸ��� ::GetLastError �Ľ����
		\note ��ʽת��˵����������ǰ�Ա�������Ӱ������
		\sa LCL_Trace_Win32E
		*/
		//@{
#	define LCL_WrapCallWin32_Trace(_fn, ...) \
	[&](const char* msg) LB_NONNULL(1){ \
		const auto res(_fn(__VA_ARGS__)); \
	\
		if(LB_UNLIKELY(!res)) \
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
		//! \brief ���ع��̵�ַ�õ��Ĺ������͡�
		using ModuleProc = std::remove_reference_t<decltype(*::GetProcAddress(::HMODULE(), {})) > ;

		/*!
		\brief ��ģ�����ָ�����̵�ָ�롣
		\pre �����ǿա�
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
		\brief �������� WinAPI �����̡�
		\note Ϊ�������壬Ӧ�ڷ�ȫ�������ռ���ʹ�ã�ע�������������Ʋ���˳����������塣
		\note ��û�к�ʽ����ʱ��ָ��ģ����ء�
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
			\brief �ֲ��洢ɾ������
			*/
		struct LB_API LocalDelete
		{
			using pointer = ::HLOCAL;

			void
				operator()(pointer) const lnothrow;
		};


		//@{
		/*!
		\brief ����Ȩ�ޡ�
		\see https://msdn.microsoft.com/en-us/library/windows/desktop/aa374892(v=vs.85).aspx ��
		\see https://msdn.microsoft.com/en-us/library/windows/desktop/aa374896(v=vs.85).aspx ��
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

		//! \brief �ļ��ض��ķ���Ȩ�ޡ�
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


			//! \brief �ļ�����Ȩ�ޡ�
			using FileAccessRights
			= leo::enum_union<AccessRights, FileSpecificAccessRights>;

		//! \relates FileAccessRights
		DefBitmaskOperations(FileAccessRights,
			leo::wrapped_enum_traits_t<FileAccessRights>)


			//! \brief �ļ�����ģʽ��
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


			//! \brief �ļ�����ѡ�
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
		//! \see https://msdn.microsoft.com/en-us/library/gg258117(v=vs.85).aspx ��
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
			//! \warning �� MSDN �ĵ�������
			EA = 0x40000,
			Invalid = INVALID_FILE_ATTRIBUTES
		};

		//! \see https://msdn.microsoft.com/en-us/library/aa363858(v=vs.85).aspx ��
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
			// \see https://msdn.microsoft.com/zh-cn/library/windows/desktop/aa365150(v=vs.85).aspx ��
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
		\brief �ж� \c FileAttributes �Ƿ�ָ��Ŀ¼��
		*/
		lconstfn PDefH(bool, IsDirectory, FileAttributes attr) lnothrow
			ImplRet(attr & Directory)
			/*!
			\brief �ж� \c ::WIN32_FIND_DATAA ָ���Ľڵ��Ƿ�ΪĿ¼��
			*/
			inline PDefH(bool, IsDirectory, const ::WIN32_FIND_DATAA& d) lnothrow
			ImplRet(IsDirectory(FileAttributes(d.dwFileAttributes)))
			/*!
			\brief �ж� \c ::WIN32_FIND_DATAW ָ���Ľڵ��Ƿ�ΪĿ¼��
			*/
			inline PDefH(bool, IsDirectory, const ::WIN32_FIND_DATAW& d) lnothrow
			ImplRet(IsDirectory(FileAttributes(d.dwFileAttributes)))

			/*!
			\throw Win32Exception ����ʧ�ܡ�
			*/
			//@{
			/*!
			\brief ���򿪵��ļ��������ڵ�������������
			\return ָ�����Ϊ�ַ����ܵ���δ֪���
			*/
			LF_API platform::NodeCategory
			TryCategorizeNodeAttributes(UniqueHandle::pointer);

		/*!
		\brief ���򿪵��ļ��������ڵ�������豸���
		\return ָ�����Ϊ�ַ����ܵ���δ֪���
		*/
		LF_API platform::NodeCategory
			TryCategorizeNodeDevice(UniqueHandle::pointer);
		//@}

		/*!
		\return ָ����Ŀ¼�򲻱�֧�ֵ��ؽ�����ǩʱΪ NodeCategory::Empty ��
		����Ϊ��Ӧ��Ŀ¼���ؽ�����ǩ����Ͻڵ����
		\todo ��Ŀ¼���Ӻͷ������ӵ��ؽ�����ǩ�ṩ�ʵ�ʵ�֡�
		*/
		//@{
		//! \brief �� FileAttributes ���ؽ�����ǩ����ڵ����
		LF_API platform::NodeCategory
			CategorizeNode(FileAttributes, unsigned long = 0) lnothrow;
		//! \brief �� \c ::WIN32_FIND_DATAA ����ڵ����
		inline PDefH(platform::NodeCategory, CategorizeNode,
			const ::WIN32_FIND_DATAA& d) lnothrow
			ImplRet(CategorizeNode(FileAttributes(d.dwFileAttributes), d.dwReserved0))
			/*!
			\brief �� \c ::WIN32_FIND_DATAW ����ڵ����
			*/
			inline PDefH(platform::NodeCategory, CategorizeNode,
				const ::WIN32_FIND_DATAW& d) lnothrow
			ImplRet(CategorizeNode(FileAttributes(d.dwFileAttributes), d.dwReserved0))
			//@}
			/*!
			\brief ���򿪵��ļ��������ڵ����
			\return ָ�������ʱΪ NodeCategory::Invalid ��
			����Ϊ�ļ����Ժ��豸����ѯ��λ������
			\sa TryCategorizeNodeAttributes
			\sa TryCategorizeNodeDevice
			*/
			LF_API platform::NodeCategory
			CategorizeNode(UniqueHandle::pointer) lnothrow;


		/*!
		\brief ������򿪶�ռ���ļ����豸��
		\pre ��Ӷ��ԣ�·�������ǿա�
		\note ���� \c ::CreateFileW ʵ�֡�
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
			\brief �ж��Ƿ��� Wine ���������С�
			\note ��� \c HKEY_CURRENT_USER �� \c HKEY_LOCAL_MACHINE
			�µ� <tt>Software\Wine</tt> ��ʵ�֡�
			*/
			LF_API bool
			CheckWine();


		/*!
		\brief �ļ�ϵͳĿ¼����״̬��
		\warning ����������
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
			\brief ������ʼ��Ŀ¼���ơ�
			\invariant <tt>dir_name.length() > 1
			&& leo::ends_with(dir_name, L"\\*")</tt> ��
			\sa GetDirName
			*/
			wstring dir_name;
			//! \brief Win32 �������ݡ�
			::WIN32_FIND_DATAW find_data;
			/*!
			\brief ���ҽڵ㡣
			*/
			unique_ptr_from<Deleter> p_node{};

		public:
			/*!
			\brief ���죺ʹ��ָ����Ŀ¼·����
			\pre ��Ӷ��ԣ�·������������ָ��ǿա�
			\throw std::system_error ָ����·������Ŀ¼��
			\throw Win32Exception ·�����Բ�ѯʧ�ܡ�

			�� UTF-16 ·��ָ����Ŀ¼��
			Ŀ¼·�����ӽ�β��б�ܺͷ�б�ܡ� ȥ����βб�ܺͷ�б�ܺ���Ϊ������Ϊ��ǰ·����
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
			//! \brief �����������ҽڵ����ǿ���رղ���״̬��
			DefDeDtor(DirectoryFindData)

				DefBoolNeg(explicit, p_node.get())

				DefGetter(const lnothrow, unsigned long, Attributes,
					find_data.dwFileAttributes)
				/*!
				\brief ���ص�ǰ������Ŀ����
				\return ��ǰ������Ŀ����
				*/
				LB_PURE DefGetter(const lnothrow,
					const wchar_t*, EntryName, find_data.cFileName)
				DefGetter(const lnothrow, const ::WIN32_FIND_DATAW&, FindData, find_data)
				DefGetter(const lnothrow, const wstring&, DirName, (LAssert(
					dir_name.length() > 1 && leo::ends_with(dir_name, L"\\*"),
					"Invalid directory name found."), dir_name))
				/*!
				\brief ȡ�ӽڵ�����͡�
				\return ��ǰ�ڵ���Ч����ҵ���Ŀ����Ϊ��ʱΪ platform::NodeCategory::Empty ��
				����Ϊ�����ļ���Ϣ�õ���ֵ��
				\sa CategorizeNode

				�����ļ���Ϣʱ�����ȵ��� CategorizeNode �Բ������ݹ��ࣻ
				Ȼ�������ָ�����ļ���һ������ CategorizeNode �Դ򿪵��ļ��жϹ��ࡣ
				*/
				platform::NodeCategory
				GetNodeCategory() const lnothrow;

			/*!
			\brief ��ȡ��������ǰ����״̬��
			\throw Win32Exception ��ȡʧ�ܡ�
			\return ������������ڵ����ļ����ǿա�

			�����ҽڵ����ǿ��������ǰ����״̬������һ���ļ�ϵͳ�
			������ҽڵ���Ϊ�ջ����ʧ����رղ���״̬���ò��ҽڵ����ա�
			���ղ��ҽڵ�ǿ�ʱ�����¼��ǰ���ҵ���Ŀ״̬��
			*/
			bool
				Read();

			//! \brief ��λ����״̬�������ҽڵ����ǿ���رղ���״̬���ò��ҽڵ����ա�
			void
				Rewind() lnothrow;
		};

		/*!
		\brief �ؽ��������ݡ�
		\note ����ֱ��ʹ��ָ��ת������ʽ��ͬ������ʱ����δ������Ϊ��
		\warning ����������
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
			\brief �������ඨ����Ĭ��ʵ�֡�
			\note ���� Data ��Ϊ����������ʹ�á�
			*/
			~ReparsePointData();

			DefGetter(const lnothrow, Data&, , pun.get())
		};


		/*!
		\brief ��ȡ�ؽ��������ݡ�
		\pre ���ԣ������ǿա�
		\exception Win32Exception ���ļ�ʧ�ܡ�
		\throw std::invalid_argument �򿪵��ļ������ؽ����㡣
		\throw std::system_error �ؽ�������ʧ�ܡ�
		\li std::errc::not_supported �ؽ������ǩ����֧�֡�
		*/
		//@{
		LF_API LB_NONNULL(1) wstring
			ResolveReparsePoint(const wchar_t*);
		LF_API LB_NONNULL(1) wstring_view
			ResolveReparsePoint(const wchar_t*, ReparsePointData::Data&);
		//@}

		/*!
		\brief չ���ַ����еĻ���������
		\pre ��Ӷ��ԣ������ǿա�
		\throw Win32Exception ����ʧ�ܡ�
		*/
		LF_API LB_NONNULL(1) wstring
			ExpandEnvironmentStrings(const wchar_t*);


		/*!
		\see https://msdn.microsoft.com/zh-cn/library/windows/desktop/aa363788(v=vs.85).aspx ��
		*/
		//@{
		//! \brief �ļ���ʶ��
		using FileID = std::uint64_t;
		//! \brief �����кš�
		using VolumeID = std::uint32_t;
		//@}

		//! \throw Win32Exception �����ļ����ѯ�ļ�Ԫ����ʧ�ܡ�
		//@{
		//@{
		//! \brief ��ѯ�ļ���������
		//@{
		LF_API size_t
			QueryFileLinks(UniqueHandle::pointer);
		/*!
		\pre ��Ӷ��ԣ�·�������ǿա�
		\note ��������ʾ�����ؽ����㡣
		*/
		LF_API LB_NONNULL(1) size_t
			QueryFileLinks(const wchar_t*, bool = {});
		//@}

		/*!
		\brief ��ѯ�ļ���ʶ��
		\return ����ʶ�;����ļ��ı�ʶ�Ķ�Ԫ�顣
		\bug ReFS �ϲ���֤Ψһ��
		\see https://msdn.microsoft.com/zh-cn/library/windows/desktop/aa363788(v=vs.85).aspx ��
		*/
		//@{
		LF_API pair<VolumeID, FileID>
			QueryFileNodeID(UniqueHandle::pointer);
		/*!
		\pre ��Ӷ��ԣ�·�������ǿա�
		\note ��������ʾ�����ؽ����㡣
		*/
		LF_API LB_NONNULL(1) pair<VolumeID, FileID>
			QueryFileNodeID(const wchar_t*, bool = {});
		//@}
		//@}

		/*!
		\brief ��ѯ�ļ���С��
		\throw std::invalid_argument ��ѯ�ļ��õ��Ĵ�СС�� 0 ��
		*/
		//@{
		LF_API std::uint64_t
			QueryFileSize(UniqueHandle::pointer);
		//! \pre ��Ӷ��ԣ�·�������ǿա�
		LF_API LB_NONNULL(1) std::uint64_t
			QueryFileSize(const wchar_t*);
		//@}

		/*
		\note ������������ѡ��ָ��Ϊ��ʱ���ԡ�
		\note ��߾���ȡ�����ļ�ϵͳ��
		*/
		//@{
		//! \brief ��ѯ�ļ��Ĵ��������ʺ�/���޸�ʱ�䡣
		//@{
		/*!
		\pre �ļ������Ϊ \c INVALID_HANDLE_VALUE ��
		�Ҿ��� AccessRights::GenericRead Ȩ�ޡ�
		*/
		LF_API void
			QueryFileTime(UniqueHandle::pointer, ::FILETIME* = {}, ::FILETIME* = {},
				::FILETIME* = {});
		/*!
		\pre ��Ӷ��ԣ�·�������ǿա�
		\note ��ʹ��ѡ������Ϊ��ָ��ʱ�Է����ļ�����������ʾ�����ؽ����㡣
		*/
		LF_API LB_NONNULL(1) void
			QueryFileTime(const wchar_t*, ::FILETIME* = {}, ::FILETIME* = {},
				::FILETIME* = {}, bool = {});
		//@}

		/*!
		\brief �����ļ��Ĵ��������ʺ�/���޸�ʱ�䡣
		*/
		//@{
		/*!
		\pre �ļ������Ϊ \c INVALID_HANDLE_VALUE ��
		�Ҿ��� FileSpecificAccessRights::WriteAttributes Ȩ�ޡ�
		*/
		LF_API void
			SetFileTime(UniqueHandle::pointer, ::FILETIME* = {}, ::FILETIME* = {},
				::FILETIME* = {});
		/*!
		\pre ��Ӷ��ԣ�·�������ǿա�
		\note ��ʹ��ѡ������Ϊ��ָ��ʱ�Է����ļ�����������ʾ�����ؽ����㡣
		*/
		LF_API LB_NONNULL(1) void
			SetFileTime(const wchar_t*, ::FILETIME* = {}, ::FILETIME* = {},
				::FILETIME* = {}, bool = {});
		//@}
		//@}
		//@}

		/*!
		\throw std::system_error ����ʧ�ܡ�
		\li std::errc::not_supported �����ʱ���ʾ����ʵ��֧�֡�
		*/
		//@{
		/*!
		\brief ת���ļ�ʱ��Ϊ�� POSIX ��Ԫ��ʼ������ʱ������
		*/
		LF_API std::chrono::nanoseconds
			ConvertTime(const ::FILETIME&);
		/*!
		\brief ת���� POSIX ��Ԫ��ʼ������ʱ����Ϊ�ļ�ʱ�䡣
		*/
		LF_API::FILETIME
			ConvertTime(std::chrono::nanoseconds);
		//@}


		/*!
		\pre �ļ������Ϊ \c INVALID_HANDLE_VALUE ��
		�Ҿ��� AccessRights::GenericRead �� AccessRights::GenericWrite Ȩ�ޡ�
		*/
		//@{
		/*!
		\brief �����ļ���
		\note ���ڴ�ӳ���ļ�ΪЭͬ���������ļ�Ϊǿ������
		\note �ڶ��͵�������ָ���ļ�������Χ����ʼƫ�����ʹ�С��
		\note ������������ֱ��ʾ�Ƿ�Ϊ��ռ�����Ƿ����̷��ء�
		*/
		//@{
		//! \throw Win32Exception ����ʧ�ܡ�
		void
			LockFile(UniqueHandle::pointer, std::uint64_t = 0,
				std::uint64_t = std::uint64_t(-1), bool = true, bool = {});

		bool
			TryLockFile(UniqueHandle::pointer, std::uint64_t = 0,
				std::uint64_t = std::uint64_t(-1), bool = true, bool = true) lnothrow;
		//@}

		/*!
		\brief �����ļ���
		\pre �ļ��ѱ�������
		*/
		bool
			UnlockFile(UniqueHandle::pointer, std::uint64_t = 0,
				std::uint64_t = std::uint64_t(-1)) lnothrow;
		//@}

		/*!
		\brief ȡϵͳĿ¼·����
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
			//! \invariant <tt>bool(p_locked)</tt> ��
			void* p_locked;

		public:
			/*!
			\brief ���죺�����洢��
			\throw Win32Exception ::GlobalLock ����ʧ�ܡ�
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
