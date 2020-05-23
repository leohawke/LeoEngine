#pragma once

#include "ldef.h"
#include <string>
#include <type_traits>

namespace leo
{
	template<typename _tString>
	//字符串特征
	struct string_traits
	{
		using string_type = std::decay_t < _tString >;
		using value_type = std::remove_cv_t<std::remove_reference_t<decltype(std::declval<string_type>()[0]) >>;
		using traits_type = typename std::char_traits < value_type >;
		//! \since build 1.4
		//@{
		using size_type = typename string_type::size_type;
		using difference_type
			= typename string_type::difference_type;
		using reference = value_type&;
		using const_reference = const value_type&;
		//@}
		using pointer = value_type*;
		using const_pointer = const value_type*;
		using initializer = std::initializer_list < value_type >;
	};

	/*!
	\note 使用 ADL 访问字符串范围。
	\note 同 std::begin 和 std::end ，但字符数组除外。
	\note 此处 string_end 语义和 boost::end 相同，但对数组类型不同于 std::end 。
	\bug decltype 指定的返回类型不能使用 ADL 。
	\see WG21 N3936 20.4.7[iterator.range] 。
	\since build 1.4
	*/
	//@{
	template<class _tRange>
	lconstfn auto
		string_begin(_tRange& c) -> decltype(c.begin())
	{
		return begin(c);
	}
	template<class _tRange>
	lconstfn auto
		string_begin(const _tRange& c) -> decltype(c.begin())
	{
		return begin(c);
	}
	//! \since build 1.4
	//@{
	template<typename _tChar>
	lconstfn _tChar*
		string_begin(_tChar* str) lnothrow
	{
		return lconstraint(str), str;
	}
#if __cplusplus <= 201402L
	//! \see http://wg21.cmeerw.net/cwg/issue1591 。
	template<typename _tElem>
	lconstfn auto
		string_begin(std::initializer_list<_tElem> il) -> decltype(il.begin())
	{
		return il.begin();
	}
#endif
	//@}

	template<class _tRange>
	lconstfn auto
		string_end(_tRange& c) -> decltype(c.end())
	{
		return end(c);
	}
	template<class _tRange>
	lconstfn auto
		string_end(const _tRange& c) -> decltype(c.end())
	{
		return end(c);
	}
	//! \since build 1.4
	//@{
	template<typename _tChar>
	lconstfn _tChar*
		string_end(_tChar* str) lnothrow
	{
		return str + ntctslen(str);
	}
#if __cplusplus <= 201402L
	//! \see http://wg21.cmeerw.net/cwg/issue1591 。
	template<typename _tElem>
	lconstfn auto
		string_end(std::initializer_list<_tElem> il) -> decltype(il.end())
	{
		return il.end();
	}
#endif
	//@}
	//@}

	template<typename _tString, typename _tChar = typename string_traits<_tString>::value_type>
	void to_lower(_tString&& str) {
		std::transform(string_begin(str), string_end(str), string_begin(str), [](_tChar c)
			{
				return static_cast<_tChar>(std::tolower(c));
			}
		);
	}
}