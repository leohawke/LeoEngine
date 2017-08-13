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
	\brief �ж��ַ������Ƿ� ISO C++ ָ���ṩ <tt>std::char_traits</tt> ���ػ���
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
	\brief ָ���� \c wchar_t �����滻�洢�ķ� \c char �ڽ��ַ����͡�
	\warning ��ͬ���͵ķǿ��ַ���ֵ�Ƿ�����滻ȡ����ʵ�ֶ��塣
	\note ���������������ͣ�Ϊ \c char16_t �� \c char32_t ֮һ������Ϊ \c void ��
	*/
	using uchar_t = cond_t<is_trivially_replaceable<wchar_t, char16_t>, char16_t,
		cond_t<is_trivially_replaceable<wchar_t, char32_t>, char32_t, void>>;

	/*!
	\brief ʹ�� <tt>std::char_traits::eq</tt> �ж��Ƿ�Ϊ���ַ���
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
	  \brief �� NTCTS ������
	  \note NTCTS(null-terminated character string) �����ַ���ǽ������ַ�����
	  ���˽����ַ���û�п��ַ���
	  \note ��ָ������ NTMBS(null-terminated mutibyte string) �����ȿ��ַ����ǡ�
	  \see ISO C++03 (17.1.12, 17.3.2.1.3.2) ��
	  */
	  //@{
	  /*!
	  \brief ����� NTCTS ���ȡ�
	  \pre ���ԣ� <tt>s</tt> ��
	  \note ����ͬ std::char_traits<_tChar>::length ��
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
	\brief ���㲻����ָ�����ȵļ� NTCTS ���ȡ�
	\pre ���ԣ� <tt>s</tt> ��
	\note ����ͬ std::char_traits<_tChar>::length ����������ָ��ֵ��
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
	\brief �� NTCTS ������
	\pre ָ��ָ�����ַ�����ָ��ͳ���ָ���ķ�ΧΪ NTCTS ��
	\see ISO C++03 (17.1.12, 17.3.2.1.3.2) ��

	��ָ���ָ��ͳ���ָ���ķ�Χ������Ϊ NTCTS ���ַ������н��з��޸Ĳ�����
	NTCTS(null-terminated character string) �����ַ���ǽ������ַ�����
	���˽����ַ���û�п��ַ�����ָ������ NTMBS(null-terminated mutibyte string) ��
	���ȿ��ַ����ǡ�
	*/
	//@{
	//! \pre ���ԣ� <tt>s1 && s2</tt> ��
	//@{
	//! \brief ���ֵ���Ƚϼ� NTCTS ��
	//@{
	//! \note ����ͬ std::basic_string<_tChar>::compare ��������ָ�����ȡ�
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
	//! \note ����ͬ std::basic_string<_tChar>::compare ��
	template<typename _tChar>
	LB_NONNULL(1, 2) LB_PURE int
		ntctscmp(const _tChar* s1, const _tChar* s2, size_t n) lnothrowv
	{
		return lconstraint(s1), lconstraint(s2),
			std::char_traits<_tChar>::compare(s1, s2, n);
	}
	//! \note ����ͬ std::lexicographical_compare ��
	template<typename _tChar>
	LB_NONNULL(1, 2) LB_PURE int
		ntctscmp(const _tChar* s1, const _tChar* s2, size_t n1, size_t n2) lnothrowv
	{
		return lconstraint(s1), lconstraint(s2),
			std::lexicographical_compare(s1, s1 + n1, s2, s2 + n2);
	}

	//@}

	/*!
	\brief ���ֵ���Ƚϼ� NTCTS �����Դ�Сд����
	\note ����ͬ std::basic_string<_tChar>::compare ��������ָ�����Ⱥʹ�Сд��
	ʹ���ַ����������жϽ�����
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
	\brief ���ֵ���Ƚϲ�����ָ�����ȵļ� NTCTS �����Դ�Сд����
	\note ����ͬ std::basic_string<_tChar>::compare �������Դ�Сд��
	ʹ���ַ����������жϽ�����
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

	//! \pre ���Ƶ� NTCTS �洢���ص���
	//@{
	//! \brief ���� NTCTS ��
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
	//! \brief ����ȷ��Դ���ȵ� NTCTS ��
	template<typename _tChar>
	LB_NONNULL(1, 2) _tChar*
		ntctscpy(_tChar* s1, const _tChar* s2, size_t n) lnothrowv
	{
		lconstraint(s1),
			lconstraint(s2);
		return lunseq(std::char_traits<_tChar>::copy(s1, s2, n), s1[n] = _tChar());
	}

	/*!
	\brief ���Ʋ�����ָ�����ȵ� NTCTS ��
	\note Ŀ���ַ�������ָ�����ȵĲ��ֻᱻ�����ַ���
	\warning Դ�ַ�����ָ��������û�п��ַ���Ŀ���ַ������Կ��ַ���β��
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