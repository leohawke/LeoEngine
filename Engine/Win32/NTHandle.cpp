// 这个文件部分代码是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。

#include "NTHandle.h"
#include <LBase/Debug.h>

#include <Windows.h>
#include <winternl.h>

namespace platform_ex {
	namespace Windows {
		template class UniqueHandle<Kernel::NtHandleCloser>;

		namespace Kernel {
			void NtHandleCloser::operator()(Handle hObject) const noexcept {
				const auto lStatus = ::NtClose(hObject);
				LAssert(NT_SUCCESS(lStatus), L"NtClose() 失败。");
			}
		}
	}
}