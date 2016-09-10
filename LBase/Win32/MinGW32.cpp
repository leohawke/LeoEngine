#include "Mingw32.h"
#if LFL_Win32
#include <LBase/LCoreUtilities.h>
#include <LBase/container.hpp>
#include <LBase/scope_gurad.hpp>
using namespace leo;
#endif
namespace platform_ex {
#if LFL_Win32

	inline namespace Windows {
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


		//@{
		LB_NONNULL(2) string
			MBCSToMBCS(int l, const char* str, unsigned cp_src, unsigned cp_dst)
		{
			if (cp_src != cp_dst)
			{
				const int
					w_len(::MultiByteToWideChar(cp_src, 0, Nonnull(str), l, {}, 0));

				if (w_len != 0)
				{
					wstring wstr(CheckPositiveScalar<size_t>(w_len), wchar_t());
					const auto w_str(&wstr[0]);

					::MultiByteToWideChar(cp_src, 0, str, l, w_str, w_len);
					if (l == -1 && !wstr.empty())
						wstr.pop_back();
					return WCSToMBCS({ w_str, wstr.length() }, cp_dst);
				}
				return{};
			}
			return str;
		}

		LB_NONNULL(2) wstring
			MBCSToWCS(int l, const char* str, unsigned cp)
		{
			const int
				w_len(::MultiByteToWideChar(cp, 0, Nonnull(str), l, {}, 0));

			if (w_len != 0)
			{
				wstring res(CheckPositiveScalar<size_t>(w_len), wchar_t());

				::MultiByteToWideChar(cp, 0, str, l, &res[0], w_len);
				if (l == -1 && !res.empty())
					res.pop_back();
				return res;
			}
			return{};
		}

		LB_NONNULL(2) string
			WCSToMBCS(int l, const wchar_t* str, unsigned cp)
		{
			const int r_l(::WideCharToMultiByte(cp, 0, Nonnull(str), l, {}, 0, {}, {}));

			if (r_l != 0)
			{
				string res(CheckPositiveScalar<size_t>(r_l), char());

				::WideCharToMultiByte(cp, 0, str, l, &res[0], r_l, {}, {});
				if (l == -1 && !res.empty())
					res.pop_back();
				return res;
			}
			return{};
		}
		//@}

		Win32Exception::Win32Exception(ErrorCode ec, string_view msg, RecordLevel lv)
			: Exception(int(ec), GetErrorCategory(), msg, lv)
		{
			LAssert(ec != 0, "No error should be thrown.");
		}
		Win32Exception::Win32Exception(ErrorCode ec, string_view msg, const char* fn,
			RecordLevel lv)
			: Win32Exception(ec, msg.to_string() + " @ " + Nonnull(fn), lv)
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


		string
			MBCSToMBCS(const char* str, unsigned cp_src, unsigned cp_dst)
		{
			return MBCSToMBCS(-1, str, cp_src, cp_dst);
		}
		string
			MBCSToMBCS(string_view sv, unsigned cp_src, unsigned cp_dst)
		{
			return sv.length() != 0 ? MBCSToMBCS(CheckNonnegativeScalar<int>(
				sv.length()), sv.data(), cp_src, cp_dst) : string();
		}

		wstring
			MBCSToWCS(const char* str, unsigned cp)
		{
			return MBCSToWCS(-1, str, cp);
		}
		wstring
			MBCSToWCS(string_view sv, unsigned cp)
		{
			return sv.length() != 0 ? MBCSToWCS(CheckNonnegativeScalar<int>(
				sv.length()), sv.data(), cp) : wstring();
		}

		string
			WCSToMBCS(const wchar_t* str, unsigned cp)
		{
			return WCSToMBCS(-1, str, cp);
		}
		string
			WCSToMBCS(wstring_view sv, unsigned cp)
		{
			return sv.length() != 0 ? WCSToMBCS(CheckNonnegativeScalar<int>(
				sv.length()), sv.data(), cp) : string();
		}
	}
#endif
}