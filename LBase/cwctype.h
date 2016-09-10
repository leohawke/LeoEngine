#ifndef LBase_cwctype_h
#define LBase_cwctype_h

#include "cctype.h"
#include <cwctype>

namespace stdex
{
	//@{
	/*!
	\brief 区域无关的 std::isprint 实现。
	\note 当前使用兼容 Unicode 的简单实现（参考 MUSL libc ）。
	\note 可作为替代 MSVCRT 的实现的变通（测试表明 iswprintf 和 isprint 有类似缺陷）。
	\sa https://connect.microsoft.com/VisualStudio/feedback/details/799287/isprint-incorrectly-classifies-t-as-printable-in-c-locale
	*/
	LB_API bool
		iswprint(wchar_t);

	/*!
	\brief 区域无关的 std::iswspace 实现。
	\note 当前使用 Unicode 5.2 实现（参考 newlib ）。
	\note 可作为替代 Android 2.3.1 的实现的变通（测试表明 iswspace 对个别字符可能返回 8 ）。
	*/
	LB_API bool
		iswspace(wchar_t);

	/*!
	\brief 区域无关的 std::iswgraph 实现。
	\see ISO C11 7.30.2.1.6 。
	*/
	inline bool
		iswgraph(wchar_t wc)
	{
		return !iswspace(wc) && iswprint(wc);
	}
}

#endif
