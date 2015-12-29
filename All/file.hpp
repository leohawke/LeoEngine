#pragma once
#include "ldef.h"
#include "leoint.hpp"
#include <base.h>
#include <Abstract.hpp>

#include <memory>
#include <string>
#include <functional>
#include <cstddef>

namespace leo
{
	namespace win
	{
		class File : noncopyable,ABSTRACT
		{
		public:
			enum : uint64
			{
				INVALIDE_SIZE = (uint64)-1
			};

			enum : uint32
			{
				//权限控制
				TO_READ = 0X00000001,//读
				TO_WRITE = 0X00000002,//写

				//创建行为控制<指定TO_WRITE下>
				NO_CREATE =0X00000004,//文件不存在失败
				FAIL_IF_EXISTS = 0X00000008,//文件存在失败

				//杂项
				NO_BUFFERING = 0X00000010,
				WRITE_THROUGH = 0X00000020,
				DEL_ON_CLOSE =0X00000040,
				NO_TRUNC = 0X00000080,//TO_WRITE下打开文件不请空现有内容
			};

		public:
			static std::unique_ptr<File> Open(const std::wstring &wsoPath, std::uint32_t u32Flags);

			static std::unique_ptr<File> OpenNoThrow(const std::wstring &wsoPath, std::uint32_t u32Flags);

		public:
			std::uint64_t GetSize() const;
			void Resize(std::uint64_t u64NewSize);
			void Clear();

			std::size_t Read(void *pBuffer, std::size_t uBytesToRead, std::uint64_t u64Offset) const;
			void Write(std::uint64_t u64Offset, const void *pBuffer, std::size_t uBytesToWrite);

			// 1. fnAsyncProc 总是会被执行一次，即使读取或写入操作失败；
			// 2. 如果 IO 请求不能一次完成（例如尝试在 64 位环境下一次读取超过 4GiB 的数据），将会拆分为多次进行。
			//    但是在这种情况下，只有第一次的操作是异步的并且会触发回调；
			// 3. 所有的回调函数都可以抛出异常；在这种情况下，异常将在读取或写入操作完成或失败后被重新抛出。
			// 4. 当且仅当 fnAsyncProc 成功返回且异步操作成功后 fnCompleteCallback 才会被执行。
			std::size_t Read(
				void *pBuffer, std::size_t uBytesToRead, std::uint64_t u64Offset,
				const std::function<void()> &fnAsyncProc,
				const std::function<void()> &fnCompleteCallback
				) const;
			void Write(
				std::uint64_t u64Offset, const void *pBuffer, std::size_t uBytesToWrite,
				const std::function<void()> &fnAsyncProc,
				const std::function<void()> &fnCompleteCallback
				);

			void Flush() const;
		};
	}
	namespace win
	{

		namespace file
		{
			enum class FILE_TYPE :std::uint8_t
			{
				DDS = 0,
				TGA = 1,
				OTHER_TEX_BEGIN = 2,
				OTHER_TEX_END = 10
			};
			bool FileExist(const wchar_t * filename);
			std::wstring GetDirectoryFromFileName(const wchar_t * filename);
			std::wstring GetFileNameWithoutExt(const wchar_t * filename);
			FILE_TYPE GetFileExt(const wchar_t* filename);

			bool FileExist(const std::wstring& filename);
			std::wstring GetDirectoryFromFileName(const std::wstring& filename);
			std::wstring GetFileNameWithoutExt(const std::wstring& filename);
			FILE_TYPE GetFileExt(const std::wstring& filename);
			const size_t max_path = 260;

			class TempFile
			{
			public:
				TempFile();
				TempFile(const std::wstring & path, const std::wstring & prefix);
				TempFile(const wchar_t * path, const wchar_t * prefix);
				~TempFile();
				const wchar_t* c_str() const;

				TempFile(const TempFile&) = delete;
				void operator=(const TempFile&) = delete;
				TempFile(TempFile&& rvalue) = delete;
			private:
				wchar_t mTempFileName[max_path];
			public:
				static void Path(wchar_t *path);
				static void Prefix(wchar_t *prefix);
			private:
				static const size_t max_prefix = 8;
				static wchar_t mPath[max_path];
				static wchar_t mPrefix[max_prefix];
			};
			void HuffManEncode(wchar_t * src, wchar_t * dst);
			void HuffManDecode(wchar_t * src, wchar_t * dst);
		}
	}
}