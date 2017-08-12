#include "Registry.h"
#include "../../LCLib/Platform.h"

namespace platform_ex
{
#if LFL_Win32

	namespace Windows
	{

		void
			RegistryKey::Flush()
		{
			LCL_RaiseZ_Win32E(::RegFlushKey(h_key), "RegFlushKey");
		}

		pair<unsigned long, vector<byte>>
			RegistryKey::GetRawValue(const wchar_t* name, unsigned long type) const
		{
			unsigned long size;

			LCL_RaiseZ_Win32E(::RegQueryValueExW(h_key,
				platform::Nonnull(name), {}, type == REG_NONE ? &type : nullptr, {},
				&size), "RegQueryValueExW");

			vector<byte> res(size);

			LCL_RaiseZ_Win32E(::RegQueryValueExW(h_key, name,
			{}, &type, &res[0], &size), "RegQueryValueExW");
			return { type, std::move(res) };
		}
		size_t
			RegistryKey::GetSubKeyCount() const
		{
			unsigned long res;

			LCL_RaiseZ_Win32E(::RegQueryInfoKey(h_key, {}, {}, {},
				&res, {}, {}, {}, {}, {}, {}, {}), "RegQueryInfoKey");
			return size_t(res);
		}
		vector<wstring>
			RegistryKey::GetSubKeyNames() const
		{
			const auto cnt(GetSubKeyCount());
			vector<wstring> res;

			if (cnt > 0)
			{
				// NOTE: See http://msdn.microsoft.com/en-us/library/windows/desktop/ms724872(v=vs.85).aspx .
				wchar_t name[256];

				for (res.reserve(cnt); res.size() < cnt; res.emplace_back(name))
					LCL_RaiseZ_Win32E(::RegEnumKeyExW(h_key,
						static_cast<unsigned long>(res.size()), name, {}, {}, {}, {},
						{}), "RegEnumKeyExW");
			}
			return res;
		}
		size_t
			RegistryKey::GetValueCount() const
		{
			unsigned long res;

			LCL_RaiseZ_Win32E(::RegQueryInfoKey(h_key, {}, {}, {}, {},
			{}, {}, &res, {}, {}, {}, {}), "RegQueryInfoKey");
			return size_t(res);
		}
		vector<wstring>
			RegistryKey::GetValueNames() const
		{
			const auto cnt(GetValueCount());
			vector<wstring> res;

			if (cnt > 0)
			{
				// NOTE: See http://msdn.microsoft.com/en-us/library/windows/desktop/ms724872(v=vs.85).aspx .
				wchar_t name[16384];

				for (res.reserve(cnt); res.size() < cnt; res.emplace_back(name))
					LCL_RaiseZ_Win32E(::RegEnumValueW(h_key,
						static_cast<unsigned long>(res.size()), name, {}, {}, {}, {},
						{}), "RegEnumValueW");
			}
			return res;
		}

		wstring
			FetchRegistryString(const RegistryKey& key, const wchar_t* name)
		{
			try
			{
				const auto pr(key.GetRawValue(name, REG_SZ));

				if (pr.first == REG_SZ && !pr.second.empty())
					// TODO: Improve performance?
					return leo::rtrim(wstring(reinterpret_cast<const wchar_t*>(
						&pr.second[0]), pr.second.size() / 2), wchar_t());
			}
			catch (Win32Exception&)
			{
			}
			return {};
		}

	} // namespace Windows;

#endif

}
