/*! \file Engine\WIN32\NTHandle.h
\ingroup Engine
\brief Win32 Kernel HANDLE 接口。
*/

// 这个文件部分代码是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。

#ifndef LE_WIN32_NTHANDLE_H
#define LE_WIN32_NTHANDLE_H 1

#include "Handle.h"

namespace platform_ex {
	namespace Windows {
		namespace Kernel {
			using Handle = void *;

			struct NtHandleCloser {
				lconstexpr Handle operator()() const lnoexcept {
					return nullptr;
				}
				void operator()(Handle hObject) const lnoexcept;
			};
		}

		extern template class MCF::UniqueHandle<Kernel::NtHandleCloser>;

		using UniqueNtHandle = MCF::UniqueHandle<Kernel::NtHandleCloser>;
	}
}


#endif