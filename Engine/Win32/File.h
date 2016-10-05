/*! \file Engine\WIN32\File.h
\ingroup Engine
\brief Win32 File 接口。
*/
#ifndef LE_WIN32_File_H
#define LE_WIN32_File_H 1

#include <LBase/experimental/string_view.hpp> //TODO:replace it
#include "NTHandle.h"
#include <LBase/linttype.hpp>

namespace platform_ex {
	namespace Windows {
		using namespace leo::inttype;

		// 这个代码部分是 MCF 的一部分。
		// 有关具体授权说明，请参阅 MCFLicense.txt。
		class File {
		public:
			enum : uint32 {
				// 权限控制。
				kToRead = 0x00000001,
				kToWrite = 0x00000002,

				// 创建行为控制。如果没有指定 TO_WRITE，这些选项都无效。
				kDontCreate = 0x00000004, // 文件不存在则失败。若指定该选项则 kFailIfExists 无效。
				kFailIfExists = 0x00000008, // 文件已存在则失败。

				// 共享访问权限。
				kSharedRead = 0x00000100, // 共享读权限。对于不带 kToWrite 打开的文件总是开启的。
				kSharedWrite = 0x00000200, // 共享写权限。
				kSharedDelete = 0x00000400, // 共享删除权限。

				// 杂项。
				kNoBuffering = 0x00010000,
				kWriteThrough = 0x00020000,
				kDeleteOnClose = 0x00040000,
				kDontTruncate = 0x00080000, // 默认情况下使用 kToWrite 打开文件会清空现有内容。
			};
		private:
			static UniqueNtHandle CreateFileWA(const std::experimental::wstring_view& path, uint32 flags);
		private:
			UniqueNtHandle file;

		public:
			lconstexpr File() lnoexcept
				: file()
			{
			}
			File(const std::experimental::wstring_view& path, uint32 flags)
				: file(CreateFileWA(path, flags))
			{
			}

		public:
			void *GetHandle() const lnoexcept {
				return file.Get();
			}

			bool IsOpen() const lnoexcept;
			void Open(const std::experimental::wstring_view& path, uint32 flags);
			bool OpenNothrow(const std::experimental::wstring_view& path, uint32 flags);
			void Close() lnoexcept;

			uint64 GetSize() const;
			void Resize(uint64 u64NewSize);
			void Clear();

			// 1. fnAsyncProc 总是会被执行一次，即使读取或写入操作失败；
			// 2. 所有的回调函数都可以抛出异常；在这种情况下，异常将在读取或写入操作完成或失败后被重新抛出。
			// 3. 当且仅当 fnAsyncProc 成功返回且异步操作成功后 fnCompletionCallback 才会被执行。
			std::size_t Read(void *pBuffer, std::size_t uBytesToRead, uint64 u64Offset/*,
				FunctionView<void()> fnAsyncProc = nullptr, FunctionView<void()> fnCompletionCallback = nullptr*/) const;
			std::size_t Write(uint64 u64Offset, const void *pBuffer, std::size_t uBytesToWrite/*,
				FunctionView<void()> fnAsyncProc = nullptr, FunctionView<void()> fnCompletionCallback = nullptr*/);
			void HardFlush();

			void Swap(File &rhs) lnoexcept {
				using std::swap;
				swap(file, rhs.file);
			}

		public:
			explicit operator bool() const lnoexcept {
				return IsOpen();
			}

			friend void swap(File &lhs, File &rhs) lnoexcept {
				lhs.Swap(rhs);
			}
		};
	}
}


#endif
