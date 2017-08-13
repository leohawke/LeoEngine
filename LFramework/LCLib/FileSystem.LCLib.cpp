#include "FileSystem.h"
#include "FFileIO.h"
#include "NativeAPI.h"
#include "../CHRLib/CharacterProcessing.h"
#include <LBase/ctime.h>

//	leo::is_time_no_leap_valid;
#if LFL_Win32
#	include "../Win32/LCLib/Mingw32.h"
//	platform_ex::ResolveReparsePoint, platform_ex::DirectoryFindData;
#	include <time.h> // for ::localtime_s;

namespace
{



	namespace LCL_Impl_details
	{
		using W32_CreateSymbolicLinkW_t = unsigned char __stdcall(const wchar_t*, const wchar_t*, unsigned long);
		
		template<typename... _tParams>
		decltype(std::declval<W32_CreateSymbolicLinkW_t>()(std::declval<_tParams&&>()...))
			W32_CreateSymbolicLinkW_call(_tParams&&... args)
		{
			return platform_ex::Windows::LoadProc<W32_CreateSymbolicLinkW_t>
				(L"kernel32", "CreateSymbolicLinkW")
				(std::forward<decltype(args)>(args)...);
		}
		template<typename... _tParams>
		auto CreateSymbolicLinkW(_tParams&&... args)
			-> decltype(W32_CreateSymbolicLinkW_call(std::forward<decltype(args)>(args)...))
		{
			return W32_CreateSymbolicLinkW_call(std::forward<decltype(args)>(args)...);
		}
		using platform::wcast;

		// NOTE: As %SYMBOLIC_LINK_FLAG_DIRECTORY, but with correct type.
		lconstexpr const auto SymbolicLinkFlagDirectory(1UL);

		inline PDefH(void, W32_CreateSymbolicLink, const char16_t* dst,
			const char16_t* src, unsigned long flags)
			ImplExpr(LCL_CallF_Win32(CreateSymbolicLinkW, wcast(dst), wcast(src), 1UL))

	} // namespace LCL_Impl_details;

} // unnamed namespace;

using platform_ex::MakePathStringW;
using platform_ex::Windows::DirectoryFindData;
#else
#	include <dirent.h>
#	include <LBase/scope_guard.hpp> // for leo::swap_guard;
#	include <time.h> // for ::localtime_r;

using platform_ex::MakePathStringU;
#endif

using namespace CHRLib;

namespace platform
{

	namespace
	{

#if !LFL_Win32
		using errno_guard = leo::swap_guard<int, void, decltype(errno)&>;
#endif

#if !LCL_DS
		template<typename _tChar>
		bool
			IterateLinkImpl(basic_string<_tChar>& path, size_t& n)
		{
			if (!path.empty())
				try
			{
				// TODO: Throw with context information about failed path?
				// TODO: Use preallocted object to improve performance?
				path = ReadLink(path.c_str());
				if (n != 0)
					--n;
				else
					// XXX: Enumerator
					//	%std::errc::too_many_symbolic_link_levels is
					//	commented out in MinGW-w64 configuration of
					//	libstdc++. See https://gcc.gnu.org/bugzilla/show_bug.cgi?id=71444.
					leo::throw_error(ELOOP, "ReadLink @ platform::IteratorLink");
				return true;
			}
			CatchIgnore(std::invalid_argument&)
				return {};
		}
#endif

#if LFL_Win32
		template<class _type, typename _tParam>
		_type*
			CreateDirectoryDataPtr(_tParam&& arg)
		{
			return new _type(lforward(arg));
		}
#endif

	} // unnamed namespace;

	bool
		IsDirectory(const char* path)
	{
#if LFL_Win32
		return IsDirectory(ucast(MakePathStringW(path).c_str()));
#else
		// TODO: Merge? See %platform_ex::estat in %NativeAPI.
		struct ::stat st;

		return ::stat(path, &st) == 0 && bool(Mode(st.st_mode) & Mode::Directory);
#endif
	}
	bool
		IsDirectory(const char16_t* path)
	{
#if LFL_Win32
		// TODO: Simplify?
		const auto attr(::GetFileAttributesW(wcast(path)));

		return attr != platform_ex::Windows::Invalid
			&& (platform_ex::Windows::FileAttributes(attr) & platform_ex::Windows::Directory);
#else
		return IsDirectory(MakePathString(path).c_str());
#endif
	}

	void
		CreateHardLink(const char* dst, const char* src)
	{
#if LFL_Win32
		CreateHardLink(ucast(MakePathStringW(dst).c_str()),
			ucast(MakePathStringW(src).c_str()));
#else
		// NOTE: POSIX %::link is implementation-defined to following symbolic
		//	link target.
		LCL_CallF_CAPI(, ::linkat, AT_FDCWD, Nonnull(src), AT_FDCWD, Nonnull(dst),
			AT_SYMLINK_FOLLOW);
#endif
	}
	void
		CreateHardLink(const char16_t* dst, const char16_t* src)
	{
#if LFL_Win32
		LCL_CallF_Win32(CreateHardLinkW, wcast(dst), wcast(src), {});
#else
		// TODO: To make the behavior specific and same as on platform %Win32, use
		//	%::realpath on platform %Linux, etc.
		CreateHardLink(MakePathString(dst).c_str(), MakePathString(src).c_str());
#endif
	}

	void
		CreateSymbolicLink(const char* dst, const char* src, bool is_dir)
	{
#if LFL_Win32
		using namespace platform_ex;

		CreateSymbolicLink(ucast(MakePathStringW(dst).c_str()),
			ucast(MakePathStringW(src).c_str()), is_dir);
#else
		lunused(is_dir);
		LCL_CallF_CAPI(, ::symlink, Nonnull(src), Nonnull(dst));
#endif
	}
	void
		CreateSymbolicLink(const char16_t* dst, const char16_t* src, bool is_dir)
	{
#if LFL_Win32
		LCL_Impl_details::W32_CreateSymbolicLink(dst, src,
			is_dir ? LCL_Impl_details::SymbolicLinkFlagDirectory : 0UL);
#else
		CreateSymbolicLink(MakePathString(dst).c_str(), MakePathString(src).c_str(),
			is_dir);
#endif
	}

	string
		ReadLink(const char* path)
	{
#if LFL_Win32
		// TODO: Simplify?
		return
			MakePathString(ReadLink(ucast(MakePathStringW(path).c_str())).c_str());
#else
		struct ::stat st;

		LCL_CallF_CAPI(, ::lstat, Nonnull(path), &st);
		if ((Mode(st.st_mode) & Mode::Link) == Mode::Link)
		{
			auto n(st.st_size);

			// NOTE: Some file systems like procfs on Linux are not
			//	conforming to POSIX, thus %st.st_size is not always reliable. In
			//	most cases, it is 0. Only 0 is currently supported.
			if (n >= 0)
			{
				// FIXME: Blocked. TOCTTOU access.
				if (n == 0)
					// TODO: Use %::pathconf to determine initial length instead
					//	of magic number.
					n = yimpl(1024);
				return leo::retry_for_vector<string>(size_t(n),
					[&](string& res, size_t s) -> bool {
					errno_guard gd(errno, 0);
					const auto r(::readlink(path, &res[0], size_t(n)));

					if (r < 0)
					{
						const int err(errno);

						if (err == EINVAL)
							throw std::invalid_argument("Failed reading link:"
								" Specified file is not a link.");
						YCL_RaiseZ_SysE(, err, "::readlink", lfsig);
						throw std::runtime_error(
							"Unknown error @ ::readlink.");
					}
					if (size_t(r) <= s)
					{
						res.resize(size_t(r));
						return {};
					}
					return true;
				});
			}
			throw std::invalid_argument("Invalid link size found.");
		}
		throw std::invalid_argument("Specified file is not a link.");
#endif
	}
	u16string
		ReadLink(const char16_t* path)
	{
#if LFL_Win32
		using namespace platform_ex::Windows;
		const auto sv(ResolveReparsePoint(wcast(path), ReparsePointData().Get()));

		return { sv.cbegin(), sv.cend() };
#else
		return MakePathStringU(ReadLink(MakePathString(path).c_str()));
#endif
	}

#if !LFL_DS
	bool
		IterateLink(string& path, size_t& n)
	{
		return IterateLinkImpl(path, n);
	}
	bool
		IterateLink(u16string& path, size_t& n)
	{
		return IterateLinkImpl(path, n);
	}
#endif


#if LFL_Win32
	class DirectorySession::Data final : public DirectoryFindData
	{
	public:
		using DirectoryFindData::DirectoryFindData;
	};
#else
	namespace
	{

		PDefH(::DIR*, ToDirPtr, DirectorySession::NativeHandle p)
			ImplRet(static_cast<::DIR*>(p))

	} // unnamed namespace;
#endif
	void
		DirectorySession::Deleter::operator()(pointer p) const lnothrowv
	{
#if LFL_Win32
		default_delete<Data>::operator()(p);
#else
		const auto res(::closedir(ToDirPtr(p)));

		LAssert(res == 0, "No valid directory found.");
		lunused(res);
#endif
	}


	DirectorySession::DirectorySession()
#if LFL_Win32
		: DirectorySession(u".")
#else
		: DirectorySession(".")
#endif
	{}
	DirectorySession::DirectorySession(const char* path)
#if LFL_Win32
		: dir(CreateDirectoryDataPtr<Data>(MakePathStringW(path)))
#else
		: sDirPath([](const char* p) YB_NONNULL(1) {
		const auto res(Deref(p) != char()
			? leo::rtrim(string(p), FetchSeparator<char>()) : ".");

		LAssert(res.empty() || EndsWithNonSeperator(res),
			"Invalid directory name state found.");
		return res + FetchSeparator<char>();
	}(path)),
		dir(::opendir(sDirPath.c_str()))
#endif
	{
#if !LFL_Win32
		if (!dir)
			leo::throw_error(errno, lfsig);
#endif
	}
	DirectorySession::DirectorySession(const char16_t* path)
#if LFL_Win32
		: dir(CreateDirectoryDataPtr<Data>(path))
#else
		: DirectorySession(MakePathString(path).c_str())
#endif
	{}

	void
		DirectorySession::Rewind() lnothrow
	{
		LAssert(dir, "Invalid native handle found.");
#if LFL_Win32
		Deref(dir.get()).Rewind();
#else
		::rewinddir(ToDirPtr(dir.get()));
#endif
	}


	HDirectory&
		HDirectory::operator++()
	{
#if LFL_Win32
		auto& find_data(*static_cast<DirectoryFindData*>(GetNativeHandle()));

		if (find_data.Read())
		{
			const wstring_view sv(find_data.GetEntryName());

			dirent_str = u16string(sv.cbegin(), sv.cend());
			LAssert(!dirent_str.empty(), "Invariant violation found.");
		}
		else
			dirent_str.clear();
#else
		LAssert(!p_dirent || bool(GetNativeHandle()), "Invariant violation found.");

		const errno_guard gd(errno, 0);

		if (const auto p = ::readdir(ToDirPtr(GetNativeHandle())))
			p_dirent = make_observer(p);
		else
		{
			const int err(errno);

			if (err == 0)
				p_dirent = {};
			else
				leo::throw_error(errno, lfsig);
		}
#endif
		return *this;
	}

	NodeCategory
		HDirectory::GetNodeCategory() const lnothrow
	{
		if (*this)
		{
			LAssert(bool(GetNativeHandle()), "Invariant violation found.");

#if LFL_Win32
			const auto res(Deref(static_cast<platform_ex::Windows::DirectoryFindData*>(
				GetNativeHandle())).GetNodeCategory());
#else
			using Descriptions::Warning;
			auto res(NodeCategory::Empty);

			try
			{
				auto name(sDirPath + Deref(p_dirent).d_name);
				struct ::stat st;

#	if LFL_DS
#		define YCL_Impl_lstatn ::stat
#	else
#		define YCL_Impl_lstatn ::lstat
#	endif
				// TODO: Simplify.
				// XXX: Value of %errno might be overwritten.
				if (YCL_TraceCallF_CAPI(YCL_Impl_lstatn, name.c_str(), &st) == 0)
					res |= CategorizeNode(st.st_mode);
#	if !LFL_DS
				if (bool(res & NodeCategory::Link)
					&& YCL_TraceCallF_CAPI(::stat, name.c_str(), &st) == 0)
					res |= CategorizeNode(st.st_mode);
#	endif
#	undef YCL_Impl_lstatn
			}
			CatchExpr(std::exception& e, YTraceDe(Warning, "Failed getting node "
				"category (errno = %d) @ %s: %s.", errno, lfsig, e.what()))
				CatchExpr(...,
					YTraceDe(Warning, "Unknown error @ %s.", lfsig))
#endif
				return res != NodeCategory::Empty ? res : NodeCategory::Invalid;
		}
		return NodeCategory::Empty;
	}

	HDirectory::operator string() const
	{
#if LFL_Win32
		return MakePathString(GetNativeName());
#else
		return GetNativeName();
#endif
	}
	HDirectory::operator u16string() const
	{
		return
#if LFL_Win32
			GetNativeName();
#else
			MakePathStringU(GetNativeName());
#endif
	}

	const HDirectory::NativeChar*
		HDirectory::GetNativeName() const lnothrow
	{
		return *this ?
#if LFL_Win32
			dirent_str.data() : u".";
#else
			p_dirent->d_name : ".";
#endif
	}


	namespace FAT
	{

		namespace LFN
		{

			tuple<string, string, bool>
				ConvertToAlias(const u16string& long_name)
			{
				LAssert(leo::ntctslen(long_name.c_str()) == long_name.length(),
					"Invalid long filename found.");

				string alias;
				// NOTE: Strip leading periods.
				size_t offset(long_name.find_first_not_of(u'.'));
				// NOTE: Set when the alias had to be modified to be valid.
				bool lossy(offset != 0);
				const auto check_char([&](string& str, char16_t uc) {
					int bc(std::wctob(std::towupper(wint_t(uc))));

					if (!lossy && std::wctob(wint_t(uc)) != bc)
						lossy = true;
					if (bc == ' ')
					{
						// NOTE: Skip spaces in filename.
						lossy = true;
						return;
					}
					// TODO: Optimize.
					if (bc == EOF || string(IllegalAliasCharacters).find(char(bc))
						!= string::npos)
						// NOTE: Replace unconvertible or illegal characters with
						//	underscores. See Microsoft FAT specification Section 7.4.
						lunseq(bc = '_', lossy = true);
					str += char(bc);
				});
				const auto len(long_name.length());

				for (; alias.length() < MaxAliasMainPartLength && long_name[offset] != u'.'
					&& offset != len; ++offset)
					check_char(alias, long_name[offset]);
				if (!lossy && long_name[offset] != u'.' && long_name[offset] != char16_t())
					// NOTE: More than 8 characters in name.
					lossy = true;

				auto ext_pos(long_name.rfind(u'.'));
				string ext;

				if (!lossy && ext_pos != u16string::npos && long_name.rfind(u'.', ext_pos)
					!= u16string::npos)
					// NOTE: More than one period in name.
					lossy = true;
				if (ext_pos != u16string::npos && ext_pos + 1 != len)
				{
					++ext_pos;
					for (size_t ext_len(0); ext_len < LFN::MaxAliasExtensionLength
						&& ext_pos != len; lunseq(++ext_len, ++ext_pos))
						check_char(ext, long_name[ext_pos]);
					if (ext_pos != len)
						// NOTE: More than 3 characters in extension.
						lossy = true;
				}
				return make_tuple(std::move(alias), std::move(ext), lossy);
			}

			string
				ConvertToMBCS(const char16_t* path)
			{
				// TODO: Optimize?
				ImplRet(leo::restrict_length(MakePathString({ path,
					std::min<size_t>(leo::ntctslen(path), MaxLength) }), MaxMBCSLength))
			}

			EntryDataUnit
				GenerateAliasChecksum(const EntryDataUnit* p) lnothrowv
			{
				static_assert(std::is_same<EntryDataUnit, unsigned char>::value,
					"Only unsigned char as byte is supported by checksum generation.");

				LAssertNonnull(p);
				// NOTE: The operation is an unsigned char rotate right.
				return std::accumulate(p, p + AliasEntryLength, 0,
					[](EntryDataUnit v, EntryDataUnit b) {
					return ((v & 1) != 0 ? 0x80 : 0) + (v >> 1) + b;
				});
			}

			bool
				ValidateName(string_view name) lnothrowv
			{
				LAssertNonnull(name.data());
				return std::all_of(begin(name), end(name), [](char c) lnothrow{
					// TODO: Use interval arithmetic.
					return c >= 0x20 && static_cast<unsigned char>(c) < 0xF0;
				});
			}

			void
				WriteNumericTail(string& alias, size_t k) lnothrowv
			{
				LAssert(!(MaxAliasMainPartLength < alias.length()), "Invalid alias found.");

				auto p(&alias[MaxAliasMainPartLength - 1]);

				while (k > 0)
				{
					LAssert(p != &alias[0], "Value is too large to store in the alias.");
					*p-- = '0' + k % 10;
					k /= 10;
				}
				*p = '~';
			}

		} // namespace LFN;

		std::time_t
			ConvertFileTime(Timestamp d, Timestamp t) lnothrow
		{
			struct std::tm time_parts;

			lunseq(
				time_parts.tm_hour = t >> 11,
				time_parts.tm_min = (t >> 5) & 0x3F,
				time_parts.tm_sec = (t & 0x1F) << 1,
				time_parts.tm_mday = d & 0x1F,
				time_parts.tm_mon = ((d >> 5) & 0x0F) - 1,
				time_parts.tm_year = (d >> 9) + 80,
				time_parts.tm_isdst = 0
			);
			return std::mktime(&time_parts);
		}

		pair<Timestamp, Timestamp>
			FetchDateTime() lnothrow
		{
			struct std::tm tmp;
			std::time_t epoch;

			if (std::time(&epoch) != std::time_t(-1))
			{
#if LFL_Win32
				// NOTE: The return type and parameter order differs than ISO C11
				//	library extension.
				::localtime_s(&tmp, &epoch);
#else
				::localtime_r(&epoch, &tmp);
				// FIXME: For platforms without %::(localtime_r, localtime_s).
#endif
				// NOTE: Microsoft FAT base year is 1980.
				return { leo::is_date_range_valid(tmp) ? ((tmp.tm_year - 80) & 0x7F)
					<< 9 | ((tmp.tm_mon + 1) & 0xF) << 5 | (tmp.tm_mday & 0x1F) : 0,
					leo::is_time_no_leap_valid(tmp) ? (tmp.tm_hour & 0x1F) << 11
					| (tmp.tm_min & 0x3F) << 5 | ((tmp.tm_sec >> 1) & 0x1F) : 0 };
			}
			return { 0, 0 };
		}


		void
			EntryData::CopyLFN(char16_t* str) const lnothrowv
		{
			const auto pos(LFN::FetchLongNameOffset((*this)[LFN::Ordinal]));

			LAssertNonnull(str);
			for (size_t i(0); i < LFN::EntryLength; ++i)
				if (pos + i < LFN::MaxLength - 1)
					str[pos + i]
					= leo::read_uint_le<16>(data() + LFN::OffsetTable[i]);
		}

		pair<EntryDataUnit, size_t>
			EntryData::FillNewName(string_view name,
				std::function<bool(string_view)> verify)
		{
			LAssertNonnull(name.data()),
				LAssertNonnull(verify);

			EntryDataUnit alias_check_sum(0);
			size_t entry_size;

			ClearAlias();
			if (name == ".")
			{
				SetDot(0),
					entry_size = 1;
			}
			else if (name == "..")
			{
				SetDot(0),
					SetDot(1),
					entry_size = 1;
			}
			else
			{
				const auto& long_name(CHRLib::MakeUCS2LE(name));
				const auto len(long_name.length());

				if (len < LFN::MaxLength)
				{
					auto alias_tp(LFN::ConvertToAlias(long_name));
					auto& pri(get<0>(alias_tp));
					const auto& ext(get<1>(alias_tp));
					auto alias(pri);
					const auto check(std::bind(verify, std::ref(alias)));

					if (!ext.empty())
						alias += '.' + ext;
					if ((get<2>(alias_tp) ? alias.length() : 0) == 0)
						entry_size = 1;
					else
					{
						entry_size
							= (len + LFN::EntryLength - 1) / LFN::EntryLength + 1;
						if (leo::ntctsicmp(alias.c_str(), name.data(),
							LFN::MaxAliasLength) != 0 || check())
						{
							size_t i(1);

							pri.resize(LFN::MaxAliasMainPartLength, '_');
							alias = pri + '.' + ext;
							leo::retry_on_cond(check, [&] {
								if (LB_UNLIKELY(LFN::MaxNumericTail < i))
									leo::throw_error(std::errc::invalid_argument,
										lfsig);
								LFN::WriteNumericTail(alias, i++);
							});
						}
					}
					WriteAlias(alias);
				}
				else
					leo::throw_error(std::errc::invalid_argument, lfsig);
				alias_check_sum = LFN::GenerateAliasChecksum(data());
			}
			return { alias_check_sum, entry_size };
		}

		bool
			EntryData::FindAlias(string_view name) const
		{
			const auto alias(GenerateAlias());

			return leo::ntctsicmp(Nonnull(name.data()), alias.c_str(),
				std::min<size_t>(name.length(), alias.length())) == 0;
		}

		string
			EntryData::GenerateAlias() const
		{
			if ((*this)[0] != Free)
			{
				if ((*this)[0] == '.')
					return (*this)[1] == '.' ? ".." : ".";

				// NOTE: Copy the base filename.
				bool case_info(((*this)[CaseInfo] & LFN::CaseLowerBasename) != 0);
				const auto conv([&](size_t i) {
					const auto c((*this)[i]);

					return char(case_info ? std::tolower(int(c)) : c);
				});
				string res;

				res.reserve(LFN::MaxAliasLength - 1);
				for (size_t i(0); i < LFN::MaxAliasMainPartLength
					&& (*this)[Name + i] != ' '; ++i)
					res += conv(Name + i);
				if ((*this)[Extension] != ' ')
				{
					res += '.';
					case_info = ((*this)[CaseInfo] & LFN::CaseLowerExtension) != 0;
					for (size_t i(0); i < LFN::MaxAliasExtensionLength
						&& (*this)[Extension + i] != ' '; ++i)
						res += conv(Extension + i);
				}
				return res;
			}
			return {};
		}

		void
			EntryData::SetupRoot(ClusterIndex root_cluster) lnothrow
		{
			Clear();
			ClearAlias(),
				SetDot(Name),
				SetDirectoryAttribute();
			WriteCluster(root_cluster);
		}

		void
			EntryData::WriteCluster(ClusterIndex c) lnothrow
		{
			using leo::write_uint_le;

			write_uint_le<16>(data() + Cluster, c),
				write_uint_le<16>(data() + ClusterHigh, c >> 16);
		}

		void
			EntryData::WriteAlias(const string& alias) lnothrow
		{
			size_t i(0), j(0);

			for (; j < LFN::MaxAliasMainPartLength && alias[i] != '.'
				&& alias[i] != char(); lunseq(++i, ++j))
				(*this)[j] = EntryDataUnit(alias[i]);
			while (j < LFN::MaxAliasMainPartLength)
			{
				(*this)[j] = ' ';
				++j;
			}
			if (alias[i] == '.')
				for (++i; alias[i] != char() && j < LFN::AliasEntryLength;
					lunseq(++i, ++j))
					(*this)[j] = EntryDataUnit(alias[i]);
			for (; j < LFN::AliasEntryLength; ++j)
				(*this)[j] = ' ';
		}

		void
			EntryData::WriteCDateTime() lnothrow
		{
			using leo::write_uint_le;
			const auto& date_time(FetchDateTime());

			write_uint_le<16>(data() + CTime, date_time.second),
				write_uint_le<16>(data() + CDate, date_time.first);
		}

		void
			EntryData::WriteDateTime() lnothrow
		{
			using leo::write_uint_le;
			using leo::unseq_apply;
			const auto date_time(FetchDateTime());
			const auto dst(data());

			unseq_apply([&](size_t offset) {
				write_uint_le<16>(dst + offset, date_time.first);
			}, CDate, MDate, ADate),
				unseq_apply([&](size_t offset) {
				write_uint_le<16>(dst + offset, date_time.second);
			}, CTime, MTime);
		}

		const char*
			CheckColons(const char* path)
		{
			if (const auto p_col = std::strchr(Nonnull(path), ':'))
			{
				path = p_col + 1;
				if (std::strchr(path, ':'))
					leo::throw_error(std::errc::invalid_argument, lfsig);
			}
			return path;
		}

	} // namespace FAT;

} // namespace platform;

