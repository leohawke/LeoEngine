// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// https://github.com/lhmouse/MCF/blob/master/MCF/Components/Notation.hpp
// Copyleft 2014. LH_Mouse. All wrongs reserved.


#ifndef __MCF_NOTATION_HPP__
#define __MCF_NOTATION_HPP__

#include <utility>
#include <map>
#include <initializer_list>
#include <cwchar>


namespace MCF {


	class Notation;


	class Package {
		friend class Notation;


	private:
		struct xKey {
			const wchar_t *m_pwchBegin;
			std::size_t m_uLength;
			std::wstring m_wcsPermanent;


			xKey(std::wstring &&wcsContents) _NOEXCEPT{
				m_wcsPermanent = std::move(wcsContents);
				m_pwchBegin = m_wcsPermanent.c_str();
				m_uLength = m_wcsPermanent.length();
			}
			xKey(const wchar_t *pwchBegin, std::size_t uLength) _NOEXCEPT{
				m_pwchBegin = pwchBegin;
				m_uLength = uLength;
			}


			xKey(const xKey &rhs){
					*this = rhs;
			}
			xKey(xKey &&rhs) _NOEXCEPT{
				*this = std::move(rhs);
			}


				xKey &operator=(const xKey &rhs){
					m_wcsPermanent.assign(rhs.m_pwchBegin, rhs.m_uLength);
					m_pwchBegin = m_wcsPermanent.c_str();
					m_uLength = rhs.m_uLength;
					return *this;
			}
			xKey &operator=(xKey &&rhs) _NOEXCEPT{
				if (rhs.m_wcsPermanent.empty()){
					m_wcsPermanent.assign(rhs.m_pwchBegin, rhs.m_uLength);
				}
				else {
					m_wcsPermanent = std::move(rhs.m_wcsPermanent);
				}
				m_pwchBegin = m_wcsPermanent.c_str();
				m_uLength = rhs.m_uLength;
				return *this;
			}


				bool operator<(const xKey &rhs) const _NOEXCEPT{
				if (m_uLength != rhs.m_uLength){
					return m_uLength < rhs.m_uLength;
				}
				return std::wmemcmp(m_pwchBegin, rhs.m_pwchBegin, m_uLength) < 0;
			}
		};


	private:
		std::map<xKey, Package> xm_mapPackages;
		std::map<xKey, std::wstring> xm_mapValues;


	private:
		Package() = default;


	public:
		const Package *GetPackage(const wchar_t *pwszName) const _NOEXCEPT{
			const auto it = xm_mapPackages.find(xKey(pwszName, std::wcslen(pwszName)));
			return (it == xm_mapPackages.end()) ? nullptr : &(it->second);
		}
			Package *GetPackage(const wchar_t *pwszName) _NOEXCEPT{
			const auto it = xm_mapPackages.find(xKey(pwszName, std::wcslen(pwszName)));
			return (it == xm_mapPackages.end()) ? nullptr : &(it->second);
		}
			Package *CreatePackage(const wchar_t *pwszName){
				xKey Key(pwszName, std::wcslen(pwszName));
				const auto itHint = xm_mapPackages.upper_bound(Key);
				return &(xm_mapPackages.emplace_hint(itHint, std::move(Key), Package())->second);
		}


		const std::wstring *GetValue(const wchar_t *pwszName) const _NOEXCEPT{
			const auto it = xm_mapValues.find(xKey(pwszName, std::wcslen(pwszName)));
			return (it == xm_mapValues.end()) ? nullptr : &(it->second);
		}
			std::wstring *GetValue(const wchar_t *pwszName) _NOEXCEPT{
			const auto it = xm_mapValues.find(xKey(pwszName, std::wcslen(pwszName)));
			return (it == xm_mapValues.end()) ? nullptr : &(it->second);
		}
			std::wstring *CreateValue(const wchar_t *pwszName){
				xKey Key(pwszName, std::wcslen(pwszName));
				const auto itHint = xm_mapValues.upper_bound(Key);
				return &(xm_mapValues.emplace_hint(itHint, std::move(Key), std::wstring())->second);
		}


		template<class PATH_SEGMENT_ITER>
		const Package *GetPackage(PATH_SEGMENT_ITER itSegBegin, PATH_SEGMENT_ITER itSegEnd) const _NOEXCEPT{
			auto ppkgCur = this;
			for (auto it = itSegBegin; it != itSegEnd; ++it){
				if (!(ppkgCur = ppkgCur->GetPackage(*it))){
					return nullptr;
				}
			}
			return ppkgCur;
		}
			template<class PATH_SEGMENT_ITER>
		Package *GetPackage(PATH_SEGMENT_ITER itSegBegin, PATH_SEGMENT_ITER itSegEnd) _NOEXCEPT{
			auto ppkgCur = this;
			for (auto it = itSegBegin; it != itSegEnd; ++it){
				if (!(ppkgCur = ppkgCur->GetPackage(*it))){
					return nullptr;
				}
			}
			return ppkgCur;
		}
			template<class PATH_SEGMENT_ITER>
		Package *CreatePackage(PATH_SEGMENT_ITER itSegBegin, PATH_SEGMENT_ITER itSegEnd){
			auto ppkgCur = this;
			for (auto it = itSegBegin; it != itSegEnd; ++it){
				ppkgCur = ppkgCur->CreatePackage(*it);
			}
			return ppkgCur;
		}


		template<class PATH_SEGMENT_ITER>
		const std::wstring *GetValue(PATH_SEGMENT_ITER itSegBegin, PATH_SEGMENT_ITER itSegEnd) const _NOEXCEPT{
			if (itSegBegin == itSegEnd){
				return nullptr;
			}
			auto itName = itSegEnd;
			--itName;
			const auto ppkgParent = GetPackage(itSegBegin, itName);
			if (!ppkgParent){
				return nullptr;
			}
			return ppkgParent->GetValue(*itName);
		}
			template<class PATH_SEGMENT_ITER>
		std::wstring *GetValue(PATH_SEGMENT_ITER itSegBegin, PATH_SEGMENT_ITER itSegEnd) _NOEXCEPT{
			if (itSegBegin == itSegEnd){
				return nullptr;
			}
			auto itName = itSegEnd;
			--itName;
			const auto ppkgParent = GetPackage(itSegBegin, itName);
			if (!ppkgParent){
				return nullptr;
			}
			return ppkgParent->GetValue(*itName);
		}
			template<class PATH_SEGMENT_ITER>
		std::wstring *CreateValue(PATH_SEGMENT_ITER itSegBegin, PATH_SEGMENT_ITER itSegEnd){
			if (itSegBegin == itSegEnd){
				return nullptr;
			}
			auto itName = itSegEnd;
			--itName;
			const auto ppkgParent = CreatePackage(itSegBegin, itName);
			return ppkgParent->CreateValue(*itName);
		}


		const Package *GetPackage(std::initializer_list<const wchar_t *> ilPath) const _NOEXCEPT{
			return GetPackage(ilPath.begin(), ilPath.end());
		}
			Package *GetPackage(std::initializer_list<const wchar_t *> ilPath) _NOEXCEPT{
			return GetPackage(ilPath.begin(), ilPath.end());
		}
			Package *CreatePackage(std::initializer_list<const wchar_t *> ilPath){
				return CreatePackage(ilPath.begin(), ilPath.end());
		}


		const std::wstring *GetValue(std::initializer_list<const wchar_t *> ilPath) const _NOEXCEPT{
			return GetValue(ilPath.begin(), ilPath.end());
		}
			std::wstring *GetValue(std::initializer_list<const wchar_t *> ilPath) _NOEXCEPT{
			return GetValue(ilPath.begin(), ilPath.end());
		}
			std::wstring *CreateValue(std::initializer_list<const wchar_t *> ilPath){
				return CreateValue(ilPath.begin(), ilPath.end());
		}


		void Clear() _NOEXCEPT{
			xm_mapPackages.clear();
			xm_mapValues.clear();
		}
	};


	class Notation :public Package
	{
	public:
		typedef enum {
			ERR_NONE,
			ERR_NO_VALUE_NAME,
			ERR_NO_PACKAGE_NAME,
			ERR_UNEXCEPTED_PACKAGE_CLOSE,
			ERR_EQU_EXPECTED,
			ERR_UNCLOSED_PACKAGE,
			ERR_ESCAPE_AT_EOF
		} ERROR_TYPE;


	private:
		static void xEscapeAndAppend(std::wstring &wcsAppendTo, const wchar_t *pwchBegin, std::size_t uLength);
		static std::wstring xUnescapeAndConstruct(const wchar_t *pwchBegin, std::size_t uLength);


		static void xExportPackageRecur(std::wstring &wcsAppendTo, const Package &pkgWhich, std::wstring &wcsPrefix, const wchar_t *pwchIndent, std::size_t uIndentLen);


	public:
		Notation();
		explicit Notation(const wchar_t *pwszText);
		Notation(const wchar_t *pwchText, std::size_t uLen);


	public:
		std::pair<ERROR_TYPE, const wchar_t *> Parse(const wchar_t *pwszText);
		std::pair<ERROR_TYPE, const wchar_t *> Parse(const wchar_t *pwchText, std::size_t uLen);
		std::wstring Export(const wchar_t *pwchIndent = L"\t") const;
	};


}
#endif

