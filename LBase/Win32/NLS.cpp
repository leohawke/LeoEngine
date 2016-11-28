#include "NLS.h"
#if LFL_Win32
#include <LBase/LCoreUtilities.h>
#include <LBase/container.hpp>
#include <LBase/scope_gurad.hpp>
using namespace leo;
#endif
namespace platform_ex {
#if LFL_Win32

	namespace Windows {

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