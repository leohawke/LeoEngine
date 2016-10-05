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
			static UniqueNtHandle CreateFile(const std::experimental::wstring_view& path, uint32 flags);
		};
	}
}


#endif
