#include "Mingw32.h"
#if LFL_Win32
#include <LBase/LCoreUtilities.h>
#include <LBase/container.hpp>
#include <LBase/scope_gurad.hpp>
#include <LBase/Win32/NLS.h>
using namespace leo;
#endif
namespace platform_ex {
#if LFL_Win32

	namespace Windows {
		class Win32ErrorCategory : public std::error_category
		{
		public:
			PDefH(const char*, name, ) const lnothrow override
				ImplRet("Win32Error")
				//! \since build 564
				PDefH(std::string, message, int ev) const override
				// NOTE: For Win32 a %::DWORD can be mapped one-to-one for 32-bit %int.
				ImplRet(Win32Exception::FormatMessage(ErrorCode(ev)))
		};


		Win32Exception::Win32Exception(ErrorCode ec, string_view msg, RecordLevel lv)
			: Exception(int(ec), GetErrorCategory(), msg, lv)
		{
			LAssert(ec != 0, "No error should be thrown.");
		}
		Win32Exception::Win32Exception(ErrorCode ec, string_view msg, const char* fn,
			RecordLevel lv)
			: Win32Exception(ec,string(msg) + " @ " + Nonnull(fn), lv)
		{}
		ImplDeDtor(Win32Exception)

			const std::error_category&
			Win32Exception::GetErrorCategory()
		{
			static const Win32ErrorCategory ecat{};

			return ecat;
		}

		std::string
			Win32Exception::FormatMessage(ErrorCode ec) lnothrow
		{
			return TryInvoke([=] {
				try
				{
					wchar_t* buf{};

					LFL_CallWin32F(FormatMessageW, FORMAT_MESSAGE_ALLOCATE_BUFFER
						| FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM,
						{}, ec, MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
						reinterpret_cast<wchar_t*>(&buf), 1, {});

					const auto p(unique_raw(buf, LocalDelete()));

					return WCSToMBCS(buf, unsigned(CP_UTF8));
				}
				CatchExpr(..., TraceDe(Warning, "FormatMessage failed."), throw)
			});
		}

		ModuleProc*
			LoadProc(::HMODULE h_module, const char* proc)
		{
			return reinterpret_cast<ModuleProc*>(LFL_CallWin32F(GetProcAddress, h_module, proc));//cl bug
		}

		void
			LocalDelete::operator()(pointer h) const lnothrow
		{
			// FIXME: For some platforms, no %::LocalFree available. See https://msdn.microsoft.com/zh-cn/library/windows/desktop/ms679351(v=vs.85).aspx.
			// NOTE: %::LocalFree ignores null handle value.
			if (LB_UNLIKELY(::LocalFree(h)))
				TraceDe(Warning, "Error %lu: failed calling LocalFree @ %s.",
					::GetLastError(), lfsig);
		}

	}
#endif
}