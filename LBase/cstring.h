#ifndef LBase_cstring_h
#define LBase_cstring_h 1

#include "LBase/type_pun.hpp" // for or_, is_same;
#include "LBase/cassert.h" // for lconstraint;
#include "LBase/cctype.h" // for stdex::tolower;

#include <cstring> // for std::strlen, std::strcpy, std::memchr, std::strncpy;
#include <string> // for std::char_traits;
#include <cwchar> // for std::wmemchr, std::wcscpy, std::wcsncpy;
#include <algorithm> // for std::min, std::lexicographical_compare;

namespace leo {

	/*!
	\ingroup unary_type_traits
	\brief 判断字符类型是否被 ISO C++ 指定提供 <tt>std::char_traits</tt> 的特化。
	*/
	template<typename _tChar>
	struct is_char_specialized_in_std : or_<is_same<_tChar, char>,
		is_same<_tChar, wchar_t>, is_same<_tChar, char16_t>,
		is_same<_tChar, char32_t >>
	{};

	template<typename _tChar, typename _type = void>
	using enable_if_irreplaceable_char_t = enable_if_t<not_<or_<
		is_trivially_replaceable<_tChar, char>,
		is_trivially_replaceable<_tChar, wchar_t>>>::value, _type>;

	/*!
	\brief 指定和 \c wchar_t 互相替换存储的非 \c char 内建字符类型。
	\warning 不同类型的非空字符的值是否可以替换取决于实现定义。
	\note 若存在这样的类型，为 \c char16_t 或 \c char32_t 之一，否则为 \c void 。
	*/
	using uchar_t = cond_t<is_trivially_replaceable<wchar_t, char16_t>, char16_t,
		cond_t<is_trivially_replaceable<wchar_t, char32_t>, char32_t, void>>;

	/*!
	\brief 使用 <tt>std::char_traits::eq</tt> 判断是否为空字符。
	*/
	template<typename _tChar>
	lconstfn  bool
		is_null(_tChar c)
	{
		return std::char_traits<_tChar>::eq(c, _tChar());
	}


	namespace details
	{

		template<typename _tChar>
		inline LB_PURE size_t
			ntctslen_raw(const _tChar* s, std::true_type)
		{
			return std::char_traits<_tChar>::length(s);
		}
		template<typename _tChar>
		LB_PURE size_t
			ntctslen_raw(const _tChar* s, std::false_type)
		{
			const _tChar* p(s);

			while (!leo::is_null(*p))
				++p;
			return size_t(p - s);
		}

	} // namespace details;


	  /*!	\defgroup NTCTSUtil null-terminated character string utilities
	  \brief 简单 NTCTS 操作。
	  \note NTCTS(null-terminated character string) 即空字符标记结束的字符串，
	  除了结束字符外没有空字符。
	  \note 简单指不包括 NTMBS(null-terminated mutibyte string) ，按等宽字符考虑。
	  \see ISO C++03 (17.1.12, 17.3.2.1.3.2) 。
	  */
	  //@{
	  /*!
	  \brief 计算简单 NTCTS 长度。
	  \pre 断言： <tt>s</tt> 。
	  \note 语义同 std::char_traits<_tChar>::length 。
	  */
	template<typename _tChar>
	inline LB_PURE size_t
		ntctslen(const _tChar* s)
	{
		lconstraint(s);

		return details::ntctslen_raw(s,
			typename is_char_specialized_in_std<_tChar>::type());
	}

	/*!
	\brief 计算不超过指定长度的简单 NTCTS 长度。
	\pre 断言： <tt>s</tt> 。
	\note 语义同 std::char_traits<_tChar>::length ，但不超过指定值。
	\since build 604
	*/
	//@{
	template<typename _tChar>
	LB_PURE size_t
		ntctsnlen(const _tChar* s, size_t n)
	{
		lconstraint(s);

		const auto str(s);

		while (n-- != 0 && *s)
			++s;

		return s - str;
	}
	inline LB_PURE size_t
		ntctsnlen(const char* s, size_t n)
	{
		lconstraint(s);

		const auto p(static_cast<const char*>(std::memchr(s, char(), n)));

		return p ? size_t(p - s) : n;
	}
	inline LB_PURE size_t
		ntctsnlen(const wchar_t* s, size_t n)
	{
		lconstraint(s);

		const auto p(static_cast<const wchar_t*>(std::wmemchr(s, char(), n)));

		return p ? size_t(p - s) : n;
	}
	//@}


	/*!	\defgroup NTCTSUtil null-terminated character string utilities
	\ingroup nonmodifying_algorithms
	\ingroup string_algorithms
	\brief 简单 NTCTS 操作。
	\pre 指针指定的字符串或指针和长度指定的范围为 NTCTS 。
	\see ISO C++03 (17.1.12, 17.3.2.1.3.2) 。

	以指针或指针和长度指定的范围参数作为 NTCTS 对字符串序列进行非修改操作。
	NTCTS(null-terminated character string) 即空字符标记结束的字符串，
	除了结束字符外没有空字符。简单指不包括 NTMBS(null-terminated mutibyte string) ，
	按等宽字符考虑。
	*/
	//@{
	//! \pre 断言： <tt>s1 && s2</tt> 。
	//@{
	//! \brief 按字典序比较简单 NTCTS 。
	//@{
	//! \note 语义同 std::basic_string<_tChar>::compare ，但忽略指定长度。
	//@{
	template<typename _tChar>
	LB_NONNULL(1, 2) LB_PURE int
		ntctscmp(const _tChar* s1, const _tChar* s2) lnothrowv
	{
		lconstraint(s1),
			lconstraint(s2);

		while (*s1 == *s2 && !leo::is_null(*s1))
			lunseq(++s1, ++s2);
		return int(*s1 - *s2);
	}
	//@{
	inline LB_NONNULL(1, 2) LB_PURE int
		ntctscmp(const char* s1, const char* s2) lnothrowv
	{
		lconstraint(s1),
			lconstraint(s2);
		return std::strcmp(s1, s2);
	}
	inline LB_NONNULL(1, 2) LB_PURE int
		ntctscmp(const wchar_t* s1, const wchar_t* s2) lnothrowv
	{
		lconstraint(s1),
			lconstraint(s2);
		return std::wcscmp(s1, s2);
	}
	inline LB_NONNULL(1, 2) LB_PURE int
		ntctscmp(const uchar_t* s1, const uchar_t* s2) lnothrowv
	{
		return ntctscmp(replace_cast<const wchar_t*>(s1),
			replace_cast<const wchar_t*>(s2));
	}
	//@}
	//@}
	//! \note 语义同 std::basic_string<_tChar>::compare 。
	template<typename _tChar>
	LB_NONNULL(1, 2) LB_PURE int
		ntctscmp(const _tChar* s1, const _tChar* s2, size_t n) lnothrowv
	{
		return lconstraint(s1), lconstraint(s2),
			std::char_traits<_tChar>::compare(s1, s2, n);
	}
	//! \note 语义同 std::lexicographical_compare 。
	template<typename _tChar>
	LB_NONNULL(1, 2) LB_PURE int
		ntctscmp(const _tChar* s1, const _tChar* s2, size_t n1, size_t n2) lnothrowv
	{
		return lconstraint(s1), lconstraint(s2),
			std::lexicographical_compare(s1, s1 + n1, s2, s2 + n2);
	}

	//@}

	/*!
	\brief 按字典序比较简单 NTCTS （忽略大小写）。
	\note 语义同 std::basic_string<_tChar>::compare ，但忽略指定长度和大小写，
	使用字符串结束符判断结束。
	*/
	template<typename _tChar>
	LB_NONNULL(1, 2) LB_PURE int
		ntctsicmp(const _tChar* s1, const _tChar* s2) lnothrowv
	{
		lconstraint(s1),
			lconstraint(s2);

		while (leo::tolower(*s1) == leo::tolower(*s2) && !leo::is_null(s2))
			lunseq(++s1, ++s2);
		return int(leo::tolower(*s1) - leo::tolower(*s2));
	}
	/*!
	\brief 按字典序比较不超过指定长度的简单 NTCTS （忽略大小写）。
	\note 语义同 std::basic_string<_tChar>::compare ，但忽略大小写，
	使用字符串结束符判断结束。
	*/
	template<typename _tChar>
	LB_NONNULL(1, 2) LB_PURE int
		ntctsicmp(const _tChar* s1, const _tChar* s2, size_t n) lnothrowv
	{
		lconstraint(s1),
			lconstraint(s2);

		using int_type = typename std::char_traits<_tChar>::int_type;
		int_type d(0);

		while (n-- != 0 && (d = int_type(leo::tolower(*s1))
			- int_type(leo::tolower(*s2))) == int_type(0)
			&& !leo::is_null(*s2))
			lunseq(++s1, ++s2);
		return int(d);
	}

	//! \pre 复制的 NTCTS 存储不重叠。
	//@{
	//! \brief 复制 NTCTS 。
	//@{
	template<typename _tChar>
	LB_NONNULL(1, 2) limpl(enable_if_irreplaceable_char_t<_tChar, _tChar*>)
		ntctscpy(_tChar* s1, const _tChar* s2) lnothrowv
	{
		lconstraint(s1),
			lconstraint(s2);

		const auto res(s1);

		while (!leo::is_null(*s1++ = *s2++))
			;
		return res;
	}
	inline LB_NONNULL(1, 2) char*
		ntctscpy(char* s1, const char* s2) lnothrowv
	{
		lconstraint(s1),
			lconstraint(s2);
		return std::strcpy(s1, s2);
	}
	inline LB_NONNULL(1, 2) wchar_t*
		ntctscpy(wchar_t* s1, const wchar_t* s2) lnothrowv
	{
		lconstraint(s1),
			lconstraint(s2);
		return std::wcscpy(s1, s2);
	}
	inline LB_NONNULL(1, 2) wchar_t*
		ntctscpy(uchar_t* s1, const uchar_t* s2) lnothrowv
	{
		return ntctscpy(replace_cast<wchar_t*>(s1),
			replace_cast<const wchar_t*>(s2));
	}
	template<typename _tChar,
		limpl(typename = enable_if_replaceable_t<_tChar, char>)>
		inline LB_NONNULL(1, 2) _tChar*
		ntctscpy(_tChar* s1, const _tChar* s2) lnothrowv
	{
		return leo::replace_cast<_tChar*>(leo::ntctscpy(leo::replace_cast<
			char*>(s1), leo::replace_cast<const char*>(s2)));
	}
	template<typename _tChar>
	inline LB_NONNULL(1, 2) limpl(enable_if_replaceable_t)<_tChar, wchar_t, _tChar*>
		ntctscpy(_tChar* s1, const _tChar* s2) lnothrowv
	{
		return leo::replace_cast<_tChar*>(leo::ntctscpy(leo::replace_cast<
			wchar_t*>(s1), leo::replace_cast<const wchar_t*>(s2)));
	}
	//@}
	//! \brief 复制确定源长度的 NTCTS 。
	template<typename _tChar>
	LB_NONNULL(1, 2) _tChar*
		ntctscpy(_tChar* s1, const _tChar* s2, size_t n) lnothrowv
	{
		lconstraint(s1),
			lconstraint(s2);
		return lunseq(std::char_traits<_tChar>::copy(s1, s2, n), s1[n] = _tChar());
	}

	/*!
	\brief 复制不超过指定长度的 NTCTS 。
	\note 目标字符串短于指定长度的部分会被填充空字符。
	\warning 源字符串在指定长度内没有空字符则目标字符串不以空字符结尾。
	*/
	//@{
	template<typename _tChar>
	LB_NONNULL(1, 2) limpl(enable_if_irreplaceable_char_t<_tChar, _tChar*>)
		ntctsncpy(_tChar* s1, const _tChar* s2, size_t n) lnothrowv
	{
		lconstraint(s1),
			lconstraint(s2);

		const auto res(s1);

		while (n != 0)
		{
			--n;
			if (leo::is_null(*s1++ = *s2++))
				break;
		}
		while (n-- != 0)
			*s1++ = _tChar();
		return res;
	}
	inline LB_NONNULL(1, 2) char*
		ntctsncpy(char* s1, const char* s2, size_t n) lnothrowv
	{
		lconstraint(s1),
			lconstraint(s2);
		return std::strncpy(s1, s2, n);
	}
	inline LB_NONNULL(1, 2) wchar_t*
		ntctsncpy(wchar_t* s1, const wchar_t* s2, size_t n) lnothrowv
	{
		lconstraint(s1),
			lconstraint(s2);
		return std::wcsncpy(s1, s2, n);
	}
	inline LB_NONNULL(1, 2) wchar_t*
		ntctsncpy(uchar_t* s1, const uchar_t* s2, size_t n) lnothrowv
	{
		return ntctsncpy(replace_cast<wchar_t*>(s1),
			replace_cast<const wchar_t*>(s2), n);
	}
	template<typename _tChar,
		limpl(typename = enable_if_replaceable_t<_tChar, char>)>
		inline LB_NONNULL(1, 2) _tChar*
		ntctsncpy(_tChar* s1, const _tChar* s2, size_t n) lnothrowv
	{
		return leo::replace_cast<_tChar*>(leo::ntctsncpy(leo::replace_cast<
			char*>(s1), leo::replace_cast<const char*>(s2), n));
	}
	template<typename _tChar>
	inline LB_NONNULL(1, 2) limpl(enable_if_replaceable_t)<_tChar, wchar_t, _tChar*>
		ntctsncpy(_tChar* s1, const _tChar* s2, size_t n) lnothrowv
	{
		return leo::replace_cast<_tChar*>(leo::ntctsncpy(leo::replace_cast<
			wchar_t*>(s1), leo::replace_cast<const wchar_t*>(s2), n));
	}
	//@}
	//@}
}

#endif