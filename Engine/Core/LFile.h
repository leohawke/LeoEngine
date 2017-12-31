/*! \file Core\LFile.h
\ingroup Engine
\brief 文件读取/写入 接口。
\brief 操作系统级别
*/
#ifndef LE_Core_File_H
#define LE_Core_File_H 1

#include <LFramework/LCLib/Platform.h>

#ifdef LFL_Win32
#include "../Win32/File.h"
#include <LBase/linttype.hpp>

namespace platform {
	using platform_ex::Windows::File;
	using namespace leo::inttype;

	class FileRead {
	public:
		FileRead(File const & file_)
			:file(file_)
		{}

		std::size_t Read(void* pBuffer, std::size_t uBytesToRead) {
			auto result = file.Read(pBuffer, uBytesToRead, u64Offset);
			u64Offset += result;
			return result;
		}

		template<typename type>
		type Read() {
			type Buffer;
			Read(&Buffer, sizeof(Buffer));
			return Buffer;
		}
	private:
		File const & file;
		uint64 u64Offset = 0;
	};
}

#endif




#endif