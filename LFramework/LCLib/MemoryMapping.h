/*!	\file MemoryMapping.h
\ingroup LCLib
\brief 内存映射文件。
*/


#ifndef LCLib_MemoryMapping_h_
#define LCLib_MemoryMapping_h_ 1

#include  <LFramework/LCLib/FCommon.h>
#include <LFramework/LCLib/FReference.h> // for ystdex::pseudo_output, unique_ptr,
//	default_delete;
#include <LFramework/LCLib/FFileIO.h>
#include <LBase/lmacro.h>

namespace platform
{
	//@{
	//! \brief 文件映射选项。
	enum class FileMappingOption
	{
		ReadOnly,
		ReadWrite,
		CopyOnWrite
	};


	/*!
	\brief 文件映射键类型。
	\note 用于指定映射的标识。
	\note 保证默认构造对象可比较，若与之相等时为匿名映射，否则可为共享映射。
	\note 为可移植性，可使用 const char16_t* 类型固定值（如字符串字面量转换的指针）。
	*/
	//@{
#if !LF_Hosted
	using FileMappingKey = leo::pseudo_output;
#elif LFL_Win32
	using FileMappingKey = const char16_t*;
#else
	using FileMappingKey = bool;
#endif
	//@}
	//@}


	class LF_API UnmapDelete
	{
	public:
		using pointer = byte*;

	private:
		size_t size = 0;

	public:
		DefDeCtor(UnmapDelete)
			UnmapDelete(size_t s)
			: size(s)
		{}


		void
			operator()(pointer) const lnothrow;

		DefGetter(const lnothrow, size_t, Size, size)
	};


	/*!
	\brief 只读内存映射文件。
	\note 可移植实现仅保证进程内生存期，但进程间可能共享。
	\note 对不支持内存映射的实现，使用 POSIX 文件 IO 模拟。
	\warning 转移后具有空状态。
	\warning 非虚析构。
	*/
	class LF_API MappedFile
	{
	private:
		FileMappingOption option;
		UniqueFile file{};
		unique_ptr<byte[], UnmapDelete> mapped{};

	public:
		/*!
		\brief 无参数构造：空状态。
		*/
		DefDeCtor(MappedFile)
			/*!
			\brief 构造：创建映射文件。
			\exception std::system_error 取文件大小失败。
			\exception std::invalid_argument 取文件大小结果小于 0 。
			\throw std::runtime_error 嵌套异常：映射失败。
			\throw std::system_error 文件打开失败。
			\note 非宿主平台：忽略第三参数。
			\note DS 平台：使用文件 IO 模拟。

			创建映射到已存在的文件上的映射。
			若键非空且不使用写时复制选项，创建全局可见的共享映射；
			否则创建修改仅在进程内可见的映射。
			*/
			//@{
			//! \pre 文件可读；使用可写映射时文件可写；文件长度非零。
			//@{
			//! \pre 指定的长度非零；文件包含指定的区域。
			//@{
			/*!
			\brief 以指定的区域偏移、区域长度、文件、选项和键创建映射。
			\note DS 和 POSIX 平台： 偏移截断为 \c ::off_t 。
			\pre 偏移位置按页对齐。
			*/
			explicit
			MappedFile(std::uint64_t, size_t, UniqueFile,
				FileMappingOption = FileMappingOption::ReadOnly, FileMappingKey = {});
		//! \brief 以指定的区域长度、文件、选项和键创建映射。
		//@{
	private:
		explicit
			MappedFile(pair<size_t, UniqueFile> pr, FileMappingOption opt,
				FileMappingKey key)
			: MappedFile(pr.first, std::move(pr.second), opt, key)
		{}

	public:
		explicit
			MappedFile(size_t, UniqueFile,
				FileMappingOption = FileMappingOption::ReadOnly, FileMappingKey = {});
		//@}
		//@}
		//! \exception ystdex::narrowing_error 映射的文件大小不被支持。
		//@{
		//! \brief 以指定的文件、选项和键创建映射。
		explicit
			MappedFile(UniqueFile,
				FileMappingOption = FileMappingOption::ReadOnly, FileMappingKey = {});
		//! \pre 间接断言：第一参数非空。
		explicit LB_NONNULL(2)
			MappedFile(const char*, FileMappingOption = FileMappingOption::ReadOnly,
				FileMappingKey = {});
		template<class _tString, typename... _tParams>
		explicit
			MappedFile(const _tString& filename, _tParams&&... args)
			: MappedFile(filename.c_str(), lforward(args)...)
		{}
		//@}
		//@}
		//@}
		DefDeMoveCtor(MappedFile)
			/*!
			\brief 析构：刷新并捕获所有错误，然后释放资源。
			\sa Flush
			*/
			~MappedFile();

		//@{
		DefDeMoveAssignment(MappedFile)

			DefBoolNeg(explicit, bool(file))

			DefGetter(const lnothrow, FileDescriptor, File, file.get())
			DefGetter(const lnothrow, FileMappingOption, MappingOption, option)
			//@}
			DefGetter(const lnothrow, byte*, Ptr, mapped.get())
			DefGetterMem(const lnothrow, size_t, Size, mapped.get_deleter())
			DefGetter(const lnothrow, const UniqueFile&, UniqueFile, file)

			//@{
			/*!
			\brief 刷新映射视图和文件。
			\pre 间接断言： <tt>GetPtr() && file</tt> 。
			*/
			PDefH(void, Flush, )
			ImplExpr(FlushView(), FlushFile())

			/*!
			\brief 刷新映射视图。
			\pre 断言： \c GetPtr() 。
			\note DS 平台：空操作。
			\note POSIX 平台：未指定只读视图关联的永久存储。
			*/
			void
			FlushView();

		/*!
		\brief 刷新文件。
		\pre 断言： \c file 。
		\note 非可写时忽略。
		*/
		void
			FlushFile();
		//@}
	};

} // namespace platform;

#endif

