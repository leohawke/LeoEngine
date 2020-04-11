/*!	\file FileSystem.h
\ingroup LCLib
\brief 平台相关的文件系统接口。
*/


#ifndef Framework_LCL_FileSystem_h_
#define Framework_LCL_FileSystem_h_ 1

#include <LFramework/LCLib/FContainer.h>
//	leo::decay_t, leo::rtrim, string, u16string, std::uint8_t,
//	std::uint32_t, size, pair, tuple;
#include <LFramework/LCLib/FReference.h> // for unique_ptr_from, tidy_ptr;
#include <system_error> // for std::system_error;
#include <LBase/sutility.h> // for leo::deref_self;
#include <LBase/iterator.hpp> // for leo::indirect_input_iterator;
#include <ctime> // for std::time_t;
#include  <LFramework/LCLib/Debug.h>
#include <LBase/linttype_utility.hpp> // for leo::read_uint_le;

#if !LFL_Win32 && !LFL_API_POSIXFileSystem
#	error "Unsupported platform found."
#endif

#if !LFL_Win32
struct dirent;
#endif

namespace platform
{
	//@{
	template<typename _tChar>
	lconstfn bool
		IsColon(_tChar c) lnothrow
	{
		return c == _tChar(':');
	}

	template<typename _tChar>
	inline LB_NONNULL(1) const _tChar*
		FindColon(const _tChar* p) lnothrowv
	{
		return leo::ntctschr(Nonnull(p), _tChar(':'));
	}

	//@{
	//! \note 取平台首选的路径分隔字符。
	//@{
	template<typename _tChar>
	LCL_Tag_constfn LB_STATELESS _tChar
		FetchSeparator_P(IDTag<LF_Platform_Win32>) lnothrow
	{
		return '\\';
	}
	template<typename _tChar>
	LCL_Tag_constfn LB_STATELESS _tChar
		FetchSeparator_P(IDTagBase) lnothrow
	{
		return '/';
	}

	template<typename _tChar>
	lconstfn LB_STATELESS _tChar
		FetchSeparator() lnothrow
	{
		return FetchSeparator_P<_tChar>(IDTag<LF_Platform>());
	}
	//@}

	//! \note 判断字符是否为平台支持的路径分隔符。
	//@{
	template<typename _tChar>
	LCL_Tag_constfn LB_STATELESS bool
		IsSeparator_P(IDTag<LF_Platform_Win32> tag, _tChar c) lnothrow
	{
		return c == FetchSeparator_P<_tChar>(tag) || c == _tChar('/');
	}
	template<typename _tChar>
	LCL_Tag_constfn LB_STATELESS bool
		IsSeparator_P(IDTagBase tag, _tChar c) lnothrow
	{
		return c == FetchSeparator_P<_tChar>(tag);
	}

	LCL_DefPlatformFwdTmpl(IsSeparator, IsSeparator_P)
		//@}
		//@}

		/*!
		\todo 支持非 Win32 文件路径特化。
		\sa FetchSeparator_P
		*/
		//@{
		/*!
		\pre 间接断言：指针路径参数或路径参数的数据指针非空。
		\note 一般字符串类型在此不支持，需另行重载，以避免空字符的语义问题和重载冲突。
		\todo 支持非 POSIX 文件路径特化。
		*/
		//@{
		/*!
		\brief 判断指定路径字符串是否表示一个绝对路径。
		\note 空路径不是绝对路径。
		*/
		//@{
	template<typename _tChar>
	inline LB_NONNULL(2) bool
		IsAbsolute_P(IDTag<LF_Platform_Win32> tag, const _tChar* path) lnothrowv
	{
		return platform::IsSeparator_P(tag, Deref(path))
			|| (*path != _tChar() && IsColon(*++path));
	}
	template<typename _tChar>
	inline bool
		IsAbsolute_P(IDTag<LF_Platform_Win32> tag, basic_string_view<_tChar> path)
		lnothrowv
	{
		LAssertNonnull(path.data());
		return path.length() > 1
			&& (platform::IsSeparator_P(tag, path.front()) || IsColon(path[1]));
	}
	template<typename _tChar>
	inline LB_NONNULL(2) bool
		IsAbsolute_P(IDTagBase tag, const _tChar* path) lnothrowv
	{
		return platform::IsSeparator_P(tag, Deref(path));
	}
	template<typename _tChar>
	inline bool
		IsAbsolute_P(IDTagBase tag, basic_string_view<_tChar> path) lnothrowv
	{
		LAssertNonnull(path.data());
		return !path.empty() && platform::IsSeparator_P(tag, path.front());
	}

	LCL_DefPlatformFwdTmpl(IsAbsolute, IsAbsolute_P)
		//@}

		/*!
		\brief 取指定路径的文件系统根节点名称的长度。
		\note 计入可能存在的紧随在根名称后的一个或多个文件分隔符。
		*/
		//@{
	template<typename _tChar>
	LB_NONNULL(2) size_t
		FetchRootNameLength_P(IDTag<LF_Platform_Win32> tag, const _tChar* path)
		lnothrowv
	{
		if (Deref(path) != _tChar() && IsColon(path[1]))
		{
			size_t n = 2;

			// TODO: Extract as %leo::tstr_find_first_not_of?
			while (path[n] != _tChar() && !platform::IsSeparator_P(tag, path[n]))
				++n;
			return n;
		}
		return 0;
	}
	template<typename _tChar>
	size_t
		FetchRootNameLength_P(IDTag<LF_Platform_Win32>, basic_string_view<_tChar> path)
		lnothrowv
	{
		LAssertNonnull(path.data());
		if (path.length() > 1 && IsColon(path[1]))
		{
			// XXX: Use %platform::IsSeparator_P?
			const auto
				n(path.find_first_not_of(&leo::to_array<_tChar>("/\\")[0], 2));

			return n != basic_string_view<_tChar>::npos ? n : 2;
		}
		return 0;
	}
	//! \since build 653
	template<typename _tChar>
	inline LB_NONNULL(2) size_t
		FetchRootNameLength_P(IDTagBase tag, const _tChar* path) lnothrowv
	{
		return platform::IsSeparator_P(tag, Deref(path)) ? 1 : 0;
	}
	template<typename _tChar>
	inline size_t
		FetchRootNameLength_P(IDTagBase tag, basic_string_view<_tChar> path)
		lnothrowv
	{
		LAssertNonnull(path.data());
		return !path.empty() && platform::IsSeparator_P(tag, path.front()) ? 1 : 0;
	}

	LCL_DefPlatformFwdTmpl(FetchRootNameLength, FetchRootNameLength_P)
		//@}
		//@}

		/*!
		\brief 判断路径字符串是否以非分隔符结束。
		*/
		//@{
		template<class _tString>
	lconstfn bool
		EndsWithNonSeperator_P(IDTagBase tag, const _tString& path) lnothrow
	{
		return !path.empty() && !platform::IsSeparator_P(tag, path.back());
	}

	LCL_DefPlatformFwdTmpl(EndsWithNonSeperator, EndsWithNonSeperator_P)
		//@}

		//@{
		template<class _tString>
	LCL_Tag_constfn _tString&&
		TrimTrailingSeperator_P(IDTag<LF_Platform_Win32>, _tString&& path, typename
			leo::string_traits<_tString>::const_pointer tail = &leo::to_array<
			typename leo::string_traits<_tString>::value_type>("/\\")[0]) lnothrow
	{
		return leo::rtrim(lforward(path), tail);
	}

	template<class _tString>
	LCL_Tag_constfn _tString&&
		TrimTrailingSeperator_P(IDTagBase, _tString&& path, typename
			leo::string_traits<_tString>::const_pointer tail = &leo::to_array<
			typename leo::string_traits<_tString>::value_type>("/")[0]) lnothrow
	{
		return leo::rtrim(lforward(path), tail);
	}

	LCL_DefPlatformFwdTmpl(TrimTrailingSeperator, TrimTrailingSeperator_P)
		//@}
		//@}


		/*!
		\brief 文件节点类别。
		*/
		enum class NodeCategory : std::uint_least32_t
	{
		Empty = 0,
		//! \since build 474
		//@{
		Invalid = 1 << 0,
		Regular = 1 << 1,
		//@}
		Unknown = Invalid | Regular,
		//! \since build 474
		//@{
		Device = 1 << 9,
		Block = Device,
		Character = Device | 1 << 7,
		Communicator = 2 << 9,
		FIFO = Communicator | 1 << 6,
		Socket = Communicator | 2 << 6,
		//@}
		SymbolicLink = 1 << 12,
		MountPoint = 2 << 12,
		Junction = MountPoint,
		//! \since build 474
		//@{
		Link = SymbolicLink | Junction,
		//@}
		Directory = 1 << 15,
		//! \since build 474
		//@{
		Missing = 1 << 16,
		Special = Link | Missing
		//@}
	};

	/*!
	\relates NodeCategory
	*/
	DefBitmaskEnum(NodeCategory)


		/*!
		\brief 判断指定路径是否指定目录。
		\pre 间接断言：参数非空。
		\since build 707
		*/
		//@{
		LF_API LB_NONNULL(1) bool
		IsDirectory(const char*);
	LF_API LB_NONNULL(1) bool
		IsDirectory(const char16_t*);
	//@}


	/*!
	\pre 断言：路径指针非空。
	\exception Win32Exception Win32 平台：本机 API 调用失败。
	\exception std::system_error 系统错误。
	\li std::errc::function_not_supported 操作不被文件系统支持。
	\li 可能有其它错误。
	*/
	//@{
	//! \note DS 平台：当前实现不支持替换文件系统，因此始终不支持操作。
	//@{
	//! \note 前两个参数为目标路径和源路径。
	//@{
	/*!
	\brief 创建硬链接。
	\note 源路径指定符号链接时，跟随符号链接目标。
	*/
	//@{
		LF_API LB_NONNULL(1, 2) void
		CreateHardLink(const char*, const char*);
		LF_API LB_NONNULL(1, 2) void
		CreateHardLink(const char16_t*, const char16_t*);
	//@}

	/*!
	\brief 创建符号链接。
	\note 第三参数指定是否创建目录链接。
	\note Win32 平台：成功调用需要操作系统和权限（或组策略）支持。
	\note 非 Win32 平台：忽略第三参数。
	*/
	//@{
		LF_API LB_NONNULL(1, 2) void
		CreateSymbolicLink(const char*, const char*, bool = {});
		LF_API LB_NONNULL(1, 2) void
		CreateSymbolicLink(const char16_t*, const char16_t*, bool = {});
	//@}
	//@}

	/*!
	\brief 读取链接指向的路径。
	\throw std::invalid_argument 指定的路径存在但不是连接。
	\note 支持 Windows 目录链接和符号链接，不特别区分。
	\note POSIX 平台：不同于 ::realpath ，分配合适的大小而不依赖 PATH_MAX 。自动重试
	分配足够长的字符串以支持不完全符合 POSIX 的文件系统（如 Linux 的 procfs ）
	导致文件大小为 0 时的情形。
	*/
	//@{
		LF_API LB_NONNULL(1) string
		ReadLink(const char*);
		LF_API LB_NONNULL(1) u16string
		ReadLink(const char16_t*);
	//@}
	//@}

	/*!
	\brief 迭代访问链接：替换非空路径并按需减少允许链接计数。
	\return 是否成功访问了链接。
	\throw std::system_error 系统错误：调用检查失败。
	\li std::errc::too_many_symbolic_link_levels 减少计数后等于 0 。
	\note 忽略空路径。对路径类别中立，用户需自行判断是否为绝对路径或相对路径。
	*/
	//@{
	LF_API bool
		IterateLink(string&, size_t&);
	LF_API bool
		IterateLink(u16string&, size_t&);
	//@}
	//@}


	/*!
	\brief 目录会话：表示打开的目录。
	\note 转移后状态无法通过其它操作取得。
	\warning 非虚析构。
	*/
	class LF_API DirectorySession
	{
	public:
#if LFL_Win32
		class Data;
#else
		using Data = void;
#endif
		using NativeHandle = Data*;

	private:
		class LF_API Deleter
#if LFL_Win32
			: private default_delete<Data>
#endif
		{
		public:
			using pointer = NativeHandle;

			void
				operator()(pointer) const lnothrowv;
		};

#if !LFL_Win32
	protected:
		/*!
		\brief 目录路径。
		\invariant <tt>!dir || (leo::string_length(sDirPath.c_str()) > 0 && \
		sDirPath.back() == FetchSeparator<char>())</tt> 。
		\since build 593
		*/
		string sDirPath;
#endif

	private:
		unique_ptr_from<Deleter> dir;

	public:
		/*!
		\brief 构造：打开目录路径。
		\pre 间接断言：参数非空。
		\throw std::system_error 打开失败。
		\note 路径可以一个或多个分隔符结束；结束的分隔符会被视为单一分隔符。
		\note 当路径只包含分隔符或为空字符串时视为当前目录。
		\note 实现中 Win32 使用 UCS2-LE ，其它平台使用 UTF-8 ；否则需要编码转换。
		\note Win32 平台： "/" 可能也被作为分隔符支持。
		\note Win32 平台： 前缀 "\\?\" 关闭非结束的 "/" 分隔符支持，
		且无视 MAX_PATH 限制。
		*/
		//@{
		//! \note 使用当前目录。
		DirectorySession();
		explicit LB_NONNULL(2)
			DirectorySession(const char*);
		explicit LB_NONNULL(2)
			DirectorySession(const char16_t*);
		//@}
		/*!
		\post \c !GetNativeHandle() 。
		*/
		DefDeMoveCtor(DirectorySession)
			/*!
			\brief 析构：关闭目录路径。
			*/
			DefDeDtor(DirectorySession)

			/*!
			\post \c !GetNativeHandle() 。
			*/
			DefDeMoveAssignment(DirectorySession)

			DefGetter(const lnothrow, NativeHandle, NativeHandle, dir.get())

			//! \brief 复位目录状态。
			void
			Rewind() lnothrow;
	};


	/*!
	\brief 目录句柄：表示打开的目录和内容迭代状态。
	\invariant <tt>!*this || bool(GetNativeHandle())</tt> 。
	*/
	class LF_API HDirectory final
		: private DirectorySession, private leo::deref_self<HDirectory>
	{
		friend deref_self<HDirectory>;

	public:
		using NativeChar =
#if LFL_Win32
			char16_t
#else
			char
#endif
			;
		using iterator = leo::indirect_input_iterator<HDirectory*>;

	private:
#if LFL_Win32
		u16string dirent_str;
#else
		/*!
		\brief 节点信息。
		*/
		tidy_ptr<::dirent> p_dirent{};
#endif

	public:
		/*!
		\brief 构造：使用目录路径。
		*/
		using DirectorySession::DirectorySession;
		DefDeMoveCtor(HDirectory)

			DefDeMoveAssignment(HDirectory)

			/*!
			\brief 间接操作：取自身引用。
			\note 使用 leo::indirect_input_iterator 和转换函数访问。
			*/
			using leo::deref_self<HDirectory>::operator*;

		/*!
		\brief 迭代：向后遍历。
		\exception std::system_error 读取目录失败。
		\
		*/
		HDirectory&
			operator++();

		/*!
		\brief 判断文件系统节点无效或有效性。
		*/
#if LFL_Win32
		DefBoolNeg(explicit, !dirent_str.empty())
#else
		DefBoolNeg(explicit, bool(p_dirent))
#endif

			DefCvt(const lnothrow, basic_string_view<NativeChar>, GetNativeName())
			operator string() const;
		operator u16string() const;

		/*!
		\brief 取节点状态信息确定的文件系统节点类别。
		\return 未迭代文件时为 NodeCategory::Empty ，否则为对应的其它节点类别。
		\note 不同系统支持的可能不同。
		*/
		NodeCategory
			GetNodeCategory() const lnothrow;
		/*!
		\brief 间接操作：取节点名称。
		\return 非空结果：子节点不可用时为对应类型的 \c "." ，否则为子节点名称。
		\note 返回的结果在析构和下一次迭代前保持有效。
		*/
		LB_ATTR_returns_nonnull const NativeChar*
			GetNativeName() const lnothrow;

		//! \brief 复位。
		using DirectorySession::Rewind;

		//@{
		PDefH(iterator, begin,)
			ImplRet(this)

			PDefH(iterator, end,)
			ImplRet(iterator())
			//@}
	};

	/*!
	\relates HDirectory
	\sa leo::is_undereferenceable
	*/
	inline PDefH(bool, is_undereferenceable, const HDirectory& i) lnothrow
		ImplRet(!i)


		/*!
		\brief 已知文件系统类型。
		*/
		enum class FileSystemType
	{
		Unknown,
		FAT12,
		FAT16,
		FAT32
	};


	/*!
	\brief 文件分配表接口。
	*/
	namespace FAT
	{

		using EntryDataUnit = std::uint8_t;
		using ClusterIndex = std::uint32_t;

		//@{
		//! \see Microsoft FAT Specification Section 3.1 。
		//@{
		//! \brief BIOS 参数块偏移量。
		enum BPB : size_t
		{
			BS_jmpBoot = 0,
			BS_OEMName = 3,
			BPB_BytsPerSec = 11,
			BPB_SecPerClus = 13,
			BPB_RsvdSecCnt = 14,
			BPB_NumFATs = 16,
			BPB_RootEntCnt = 17,
			BPB_TotSec16 = 19,
			BPB_Media = 21,
			BPB_FATSz16 = 22,
			BPB_SecPerTrk = 24,
			BPB_NumHeads = 26,
			BPB_HiddSec = 28,
			BPB_TotSec32 = 32
		};

		/*!
		\brief BIOS 参数块偏移量域 BPB_BytsPerSec 的最值。
		*/
		lconstexpr const size_t MinSectorSize(512), MaxSectorSize(4096);
		//@}

		//! \brief FAT16 接口（ FAT12 共享实现）。
		inline namespace FAT16
		{

			/*!
			\brief FAT12 和 FAT16 扩展 BIOS 参数块偏移量。
			\see Microsoft FAT Specification Section 3.2 。
			*/
			enum BPB : size_t
			{
				BS_DrvNum = 36,
				BS_Reserved1 = 37,
				BS_BootSig = 38,
				BS_VolID = 39,
				BS_VolLab = 43,
				BS_FilSysType = 54,
				_reserved_zero_448 = 62,
				Signature_word = 510,
				_reserved_remained = 512
			};

		} // inline namespace FAT16;

		  //! \brief FAT32 接口。
		inline namespace FAT32
		{

			/*!
			\brief FAT32 扩展 BIOS 参数块偏移量。
			\see Microsoft FAT Specification Section 3.3 。
			*/
			enum BPB : size_t
			{
				BPB_FATSz32 = 36,
				BPB_ExtFlags = 40,
				BPB_FSVer = 42,
				BPB_RootClus = 44,
				BPB_FSInfo = 48,
				BPB_BkBootSec = 50,
				BPB_Reserved = 52,
				BS_DrvNum = 64,
				BS_Reserved1 = 65,
				BS_BootSig = 66,
				BS_VolID = 67,
				BS_VolLab = 71,
				BS_FilSysType = 82,
				_reserved_zero_420 = 90
			};

		} // inline namespace FAT32;

		  /*!
		  \brief 卷标数据类型。
		  \see Microsoft FAT Specification Section 3.2 。
		  \since build 610
		  */
		using VolumeLabel = array<byte, 11>;

		//! \see Microsoft FAT Specification Section 5 。
		//@{
		//! \brief 文件系统信息块偏移量。
		enum FSI : size_t
		{
			FSI_LeadSig = 0,
			FSI_Reserved1 = 4,
			FSI_StrucSig = 484,
			FSI_Free_Count = 488,
			FSI_Nxt_Free = 492,
			FSI_Reserved2 = 496,
			FSI_TrailSig = 508
		};

		lconstexpr const std::uint32_t FSI_LeadSig_Value(0x41615252),
			FSI_StrucSig_Value(0x61417272), FSI_TrailSig_Value(0xAA550000);
		//@}
		//@}

		/*!
		\brief 文件属性。
		\see Microsoft FAT specification Section 6 。
		*/
		enum class Attribute : EntryDataUnit
		{
			ReadOnly = 0x01,
			Hidden = 0x02,
			System = 0x04,
			VolumeID = 0x08,
			Directory = 0x10,
			Archive = 0x20,
			LongFileName = ReadOnly | Hidden | System | VolumeID
		};

		//! \relates Attribute
		DefBitmaskEnum(Attribute)


			//! \brief 簇接口。
			namespace Clusters
		{

			// !\see Microsoft FAT Specification Section 3.2 。
			//@{
			lconstexpr const size_t PerFAT12(4085);
			lconstexpr const size_t PerFAT16(65525);
			//@}

			enum : ClusterIndex
			{
				FAT16RootDirectory = 0,
				/*!
				\since build 609
				\see Microsoft FAT Specification Section 4 。
				*/
				//@{
				MaxValid12 = 0xFF6,
				MaxValid16 = 0xFFF6,
				MaxValid32 = 0xFFFFFF6,
				Bad12 = 0xFF7,
				Bad16 = 0xFFF7,
				Bad32 = 0xFFFFFF7,
				EndOfFile12 = 0xFFF,
				EndOfFile16 = 0xFFFF,
				EndOfFile32 = 0xFFFFFFFF,
				//@}
				EndOfFile = 0x0FFFFFFF,
				First = 0x00000002,
				Root = 0x00000000,
				Free = 0x00000000,
				Error = 0xFFFFFFFF
			};

			inline PDefH(bool, IsFreeOrEOF, ClusterIndex c) lnothrow
				ImplRet(c == Clusters::Free || c == Clusters::EndOfFile)

		} // namespace Clusters;


		  //! \brief 非法别名字符。
		const char IllegalAliasCharacters[]{ "\\/:;*?\"<>|&+,=[] " };


		//! \brief 文件大小：字节数。
		using FileSize = std::uint32_t;

		/*!
		\brief 最大文件大小。
		\note 等于 4GiB - 1B 。
		*/
		static lconstexpr const auto MaxFileSize(FileSize(0xFFFFFFFF));


		//! \brief 时间戳：表示日期和时间的整数类型。
		using Timestamp = std::uint16_t;

		//! \brief 转换日期和时间的时间戳为标准库时间类型。
		LF_API std::time_t
			ConvertFileTime(Timestamp, Timestamp) lnothrow;

		//! \brief 取以实时时钟的文件日期和时间。
		LF_API pair<Timestamp, Timestamp>
			FetchDateTime() lnothrow;


		/*!
		\brief 长文件名接口。
		\note 仅适用于 ASCII 兼容字符集。
		\see Microsoft FAT specification Section 7 。
		*/
		namespace LFN
		{

			//! \brief 长文件名目录项偏移量。
			enum Offsets
			{
				//! \brief 长文件名序数。
				Ordinal = 0x00,
				Char0 = 0x01,
				Char1 = 0x03,
				Char2 = 0x05,
				Char3 = 0x07,
				Char4 = 0x09,
				//! \note 值等于 Attribute::LongFilename 。
				Flag = 0x0B,
				//! \note 保留值 0x00 。
				Reserved1 = 0x0C,
				//! \brief 短文件名（别名）校验和。
				CheckSum = 0x0D,
				Char5 = 0x0E,
				Char6 = 0x10,
				Char7 = 0x12,
				Char8 = 0x14,
				Char9 = 0x16,
				Char10 = 0x18,
				//! \note 保留值 0x0000 。
				Reserved2 = 0x1A,
				Char11 = 0x1C,
				Char12 = 0x1E
			};

			//! \brief 组成长文件名中的 UCS-2 字符在项中的偏移量表。
			lconstexpr const size_t OffsetTable[]{ 0x01, 0x03, 0x05, 0x07, 0x09, 0x0E,
				0x10, 0x12, 0x14, 0x16, 0x18, 0x1C, 0x1E };

			enum : size_t
			{
				//! \brief UCS-2 项最大长度。
				MaxLength = 256,
				//! \brief UTF-8 项最大长度。
				MaxMBCSLength = MaxLength * 3,
				EntryLength = size(OffsetTable),
				AliasEntryLength = 11,
				MaxAliasMainPartLength = 8,
				MaxAliasExtensionLength = 3,
				MaxAliasLength = MaxAliasMainPartLength + MaxAliasExtensionLength + 2,
				MaxNumericTail = 999999
			};

			enum EntryValues : EntryDataUnit
			{
				//! \brief WinNT 小写文件名。
				CaseLowerBasename = 0x08,
				//! \brief WinNT 小写扩展名。
				CaseLowerExtension = 0x10,
				//! \brief Ordinal 中标记结束的项。
				LastLongEntry = 0x40
			};

			//! \brief 非法长文件名字符。
			const char IllegalCharacters[]{ "\\/:*?\"<>|" };

			//@{
			/*!
			\brief 转换长文件名为别名。
			\pre 参数指定的字符串不含空字符。
			\return 主文件名、扩展名和指定是否为有损转换标识。
			\note 返回的文件名长度分别不大于 MaxAliasMainPartLength
			和 MaxAliasExtensionLength 。
			*/
			LF_API tuple<string, string, bool>
				ConvertToAlias(const u16string&);

			//! \brief 按指定序数取长文件名偏移。
			inline PDefH(size_t, FetchLongNameOffset, EntryDataUnit ord) lnothrow
				ImplRet((size_t(ord & ~LastLongEntry) - 1U) * EntryLength)
				//@}

				/*!
				\brief 转换 UCS-2 路径字符串为多字节字符串。
				*/
				LF_API string
				ConvertToMBCS(const char16_t* path);

			/*!
			\brief 生成别名校验和。
			\pre 断言：参数非空。
			\see Microsoft FAT specification Section 7.2 。
			*/
			LF_API LB_NONNULL(1) EntryDataUnit
				GenerateAliasChecksum(const EntryDataUnit*) lnothrowv;

			/*!
			\brief 检查名称是否为合法的可被 UCS-2 表示的非控制字符组成。
			\pre 断言：字符串参数的数据指针非空。
			*/
			LF_API bool
				ValidateName(string_view) lnothrowv;

			/*!
			\brief 在指定字符串写前缀字符 '~' 和整数的后缀以作为短文件名。
			\pre 字符串长度不小于 MaxAliasMainPartLength 且可保存前缀字符和第二参数转换的数值。
			*/
			LF_API void
				WriteNumericTail(string&, size_t) lnothrowv;

		} // namespace LFN;

		  //@{
		  //! \brief 目录项数据大小。
		lconstexpr const size_t EntryDataSize(0x20);

		/*!
		\brief 目录项数据。
		\note 默认构造不初始化。
		*/
		class LF_API EntryData final : private array<EntryDataUnit, EntryDataSize>
		{
		public:
			using Base = array<EntryDataUnit, EntryDataSize>;
			/*!
			\brief 目录项偏移量。
			\see Microsoft FAT specification Section 6 。
			*/
			enum Offsets : size_t
			{
				Name = 0x00,
				Extension = 0x08,
				Attributes = 0x0B,
				//! \note 项 DIR_NTRes 保留，指定值为 0 但扩展为表示大小写。
				CaseInfo = 0x0C,
				CTimeTenth = 0x0D,
				CTime = 0x0E,
				CDate = 0x10,
				ADate = 0x12,
				ClusterHigh = 0x14,
				MTime = 0x16,
				MDate = 0x18,
				Cluster = 0x1A,
				FileSize = 0x1C
			};
			//! \see Microsoft FAT specification Section 6.1 。
			enum : EntryDataUnit
			{
				Last = 0x00,
				Free = 0xE5,
			};

			using Base::operator[];
			using Base::data;

			DefPred(const lnothrow, Directory,
				bool(Attribute((*this)[Attributes]) & Attribute::Directory))
				DefPred(const lnothrow, LongFileName,
					Attribute((*this)[Attributes]) == Attribute::LongFileName)
				DefPred(const lnothrow, Volume, bool(Attribute((*this)[Attributes])
					& Attribute::VolumeID))
				DefPred(const lnothrow, Writable,
					!bool(Attribute((*this)[Attributes]) & Attribute::ReadOnly))

				PDefH(void, SetDirectoryAttribute, ) lnothrow
				ImplExpr((*this)[Attributes] = EntryDataUnit(Attribute::Directory))
				PDefH(void, SetDot, size_t n) lnothrowv
				ImplExpr(LAssert(n < EntryDataSize, "Invalid argument found."),
				(*this)[n] = '.')

				PDefH(void, Clear, ) lnothrow
				ImplExpr(leo::trivially_fill_n(static_cast<Base*>(this)))

				PDefH(void, ClearAlias, ) lnothrow
				ImplExpr(leo::trivially_fill_n(data(), LFN::AliasEntryLength, ' '))

				/*!
				\brief 复制长文件名列表项数据到参数指定的缓冲区的对应位置。
				\pre 断言：参数非空。
				*/
				void
				CopyLFN(char16_t*) const lnothrowv;

			PDefH(void, FillLast, ) lnothrow
				ImplExpr(leo::trivially_fill_n(static_cast<Base*>(this), 1, Last))

				/*!
				\brief 为添加的项填充名称数据，按需生成短名称后缀。
				\pre 断言：字符串参数的数据指针非空。
				\pre 断言：第二参数非空。
				\return 别名校验值（若不存在别名则为 0 ）和项的大小。
				\note 第二参数是校验别名项存在性的例程，其中字符串的数据指针保证非空。
				\sa LFN::GenerateAliasChecksum
				\sa LFN::WriteNumericTail
				*/
				pair<EntryDataUnit, size_t>
				FillNewName(string_view, std::function<bool(string_view)>);

			/*!
			\pre 间接断言：参数的数据指针非空。
			\since build 656
			*/
			bool
				FindAlias(string_view) const;

			string
				GenerateAlias() const;

			PDefH(std::uint32_t, ReadFileSize, ) lnothrow
				ImplRet(leo::read_uint_le<32>(data() + FileSize))

				void
				SetupRoot(ClusterIndex) lnothrow;

			void
				WriteAlias(const string&) lnothrow;

			void
				WriteCDateTime() lnothrow;

			void
				WriteCluster(ClusterIndex) lnothrow;

			void
				WriteDateTime() lnothrow;
		};
		//@}


		/*!
		\brief 检查参数指定的 MS-DOS 风格路径冒号。
		\pre 间接断言：参数非空。
		\exception std::system_error 检查失败。
		\li std::errc::invalid_argument 路径有超过一个冒号。
		*/
		LF_API LB_NONNULL(1) LB_PURE const char*
			CheckColons(const char*);

	} // namespace FAT;

} // namespace platform;

namespace platform_ex
{

} // namespace platform_ex;

#endif

