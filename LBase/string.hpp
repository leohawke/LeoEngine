/*!	\file string.hpp
\ingroup LBase
\brief ISO C++ 标准字符串扩展。
*/

#ifndef LBase_string_hpp
#define LBase_string_hpp 1

#include "LBase/memory.hpp" // for decay_t, remove_rcv_t, std::declval,
//	nested_allocator, is_enum, is_class;
#include "LBase/container.hpp" // for "container.hpp", make_index_sequence,
//	index_sequence, begin, end, sort_unique, size, underlying;
#include "LBase/cstdio.h" // for lconstraint, vfmtlen;
#include "LBase/cstring.h" // for ntctslen;
#include "LBase/array.hpp" // for std::bidirectional_iterator_tag, to_array;
#include "LBase/type_traits.hpp"
#include <istream> // for std::basic_istream;
#include "ios.hpp" // for rethrow_badstate;
#include <ostream> // for std::basic_ostream;
#include <sstream> // for std::ostringstream, std::wostring_stream;
#include <cstdarg> // for std::va_list;

namespace leo
{
	template<typename _tString>
	//字符串特征
	struct string_traits
	{
		using string_type = decay_t < _tString >;
		using value_type = remove_rcv_t < decltype(std::declval<string_type>()[0]) >;
		using traits_type = typename std::char_traits < value_type >;
		//! \since build 1.4
		//@{
		using allocator_type = _t<nested_allocator<string_type>>;
		using size_type = typename std::allocator_traits<allocator_type>::size_type;
		using difference_type
			= typename std::allocator_traits<allocator_type>::difference_type;
		using reference = value_type&;
		using const_reference = const value_type&;
		//@}
		using pointer = value_type*;
		using const_pointer = const value_type*;
		using initializer = std::initializer_list < value_type >;
	};


	//! \since build 1.3
	namespace details
	{

		//! \since build 1.4
		//@{
		template<typename _type, typename = void>
		struct is_string_like : false_type
		{};

		template<typename _type>
		struct is_string_like<_type, enable_if_t<
			is_object<decay_t<decltype(std::declval<_type>()[0])>>::value>> : true_type
		{};
		//@}


		//! \todo 支持 std::forward_iterator_tag 重载。
		template<typename _tFwd1, typename _tFwd2, typename _fPred>
		bool
			ends_with_iter_dispatch(_tFwd1 b, _tFwd1 e, _tFwd2 bt, _tFwd2 et,
				_fPred comp, std::bidirectional_iterator_tag)
		{
			auto i(e);
			auto it(et);

			while (i != b && it != bt)
				if (!comp(*--i, *--it))
					return{};
			return it == bt;
		}


		//! \since build 1.4
		//@{
		template<size_t>
		struct str_algos;

		template<>
		struct str_algos<0>
		{
			//! \since build 1.4
			template<class _tString,
				typename _tSize = typename string_traits<_tString>::size_type>
				static lconstfn auto
				erase_left(_tString& s, _tSize n) lnothrowv
				->limpl(decltype(s.remove_prefix(n), s))
			{
				return lconstraint(n < s.size() || n == _tSize(-1)),
					s.remove_prefix(n != _tSize(-1) ? n : _tSize(s.size())), s;
			}

			//! \since build 1.4
			template<class _tString,
				typename _tSize = typename string_traits<_tString>::size_type>
				static lconstfn auto
				erase_right(_tString& s, _tSize n) lnothrowv
				->limpl(decltype(s.remove_suffix(n), s))
			{
				return lconstraint(n < s.size() || n == _tSize(-1)),
					s.remove_suffix(s.size() - n - 1), s;
			}
		};

		template<>
		struct str_algos<1>
		{
			//! \since build 1.4
			template<class _tString,
				typename _tSize = typename string_traits<_tString>::size_type>
				static lconstfn auto
				erase_left(_tString& s, _tSize n) lnothrowv
				->limpl(decltype(s.erase(0, n)))
			{
				return lconstraint(n < s.size() || n == _tSize(-1)), s.erase(0, n);
			}

			//! \since build 1.4
			template<class _tString,
				typename _tSize = typename string_traits<_tString>::size_type>
				static lconstfn auto
				erase_right(_tString& s, _tSize n) lnothrowv
				->limpl(decltype(s.erase(n + 1)))
			{
				return lconstraint(n < s.size() || n == _tSize(-1)), s.erase(n + 1);
			}
		};


		template<limpl(typename = make_index_sequence<2>)>
		struct str_algo;

		template<>
		struct str_algo<index_sequence<>>
		{
			void
				erase_left() = delete;

			void
				erase_right() = delete;
		};

		template<size_t _vIdx, size_t... _vSeq>
		struct str_algo<index_sequence<_vIdx, _vSeq...>>
			: str_algos<_vIdx>, str_algo<index_sequence<_vSeq...>>
		{
			using str_algos<_vIdx>::erase_left;
			using str_algo<index_sequence<_vSeq...>>::erase_left;
			using str_algos<_vIdx>::erase_right;
			using str_algo<index_sequence<_vSeq...>>::erase_right;
		};
		//@}

	} // unnamed namespace;


	  //! \ingroup unary_type_traits
	  //@{
	  /*!
	  \brief 判断指定类型是否为类字符串类型。
	  \since build 1.4
	  */
	template<typename _type>
	struct is_string_like : details::is_string_like<_type>
	{};

	/*!
	\brief 判断指定类型是否为字符串类类型。
	\since build 1.4
	*/
	template<typename _type>
	struct is_string_class : and_<is_class<_type>, is_string_like<_type>>
	{};
	//@}

	/*!
	\ingroup metafunctions
	\brief 选择字符串类类型的特定重载避免和其它非字符串类型冲突。
	\sa enable_if_t
	\since build 1.4
	*/
	template<typename _tParam, typename _type = void>
	using enable_for_string_class_t
		= enable_if_t<is_string_class<decay_t<_tParam>>::value, _type>;

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

	/*!	\defgroup string_algorithms String Algorithms
	\addtogroup algorithms
	\brief 字符串算法。
	\note 模板形参关键字 \c class 表示仅支持类类型对象字符串。
	*/
	//@{
	/*!
	\breif 计算字符串长度。
	*/
	//@{
	template<typename _type>
	inline size_t
		string_length(const _type* str) lnothrow
	{
		return std::char_traits<_type>::length(str);
	}
	template<class _type>
	lconstfn auto
		//字符串长度
		string_length(const _type& str)->decltype(size(str))
	{
		return size(str);
	}
#if __cplusplus <= 201402L
	//! \see http://wg21.cmeerw.net/cwg/issue1591 。
	template<typename _tElem>
	lconstfn size_t
		string_length(std::initializer_list<_tElem> il)
	{
		return il.size();
	}
#endif
	//@}


	/*!
	\note 使用 ADL string_begin 和 string_end 指定范围迭代器。
	\note 除 ADL 外接口同 Boost.StringAlgo 。
	\sa leo::string_begin, leo::string_end
	\since build 1.4
	*/
	//@{
	//! \brief 判断第一个参数指定的串是否以第二个参数起始。
	//@{
	//! \since build 519
	template<typename _tRange1, typename _tRange2, typename _fPred>
	bool
		begins_with(const _tRange1& input, const _tRange2& test, _fPred comp)
	{
		return leo::string_length(test) <= leo::string_length(input)
			&& std::equal(string_begin(test), string_end(test),
				string_begin(input), comp);
	}
	//! \since build 1.4
	template<typename _tRange1, typename _tRange2>
	inline bool
		begins_with(const _tRange1& input, const _tRange2& test)
	{
		return leo::begins_with(input, test, is_equal());
	}
	//@}

	//! \brief 判断第一个参数指定的串是否以第二个参数结束
	//@{
	template<typename _tRange1, typename _tRange2, typename _fPred>
	inline bool
		ends_with(const _tRange1& input, const _tRange2& test, _fPred comp)
	{
		// NOTE: See $2014-07 @ %Documentation::Workflow::Annual2014.
		return details::ends_with_iter_dispatch(string_begin(input),
			string_end(input), string_begin(test), string_end(test), comp, typename
			std::iterator_traits<remove_reference_t<decltype(string_begin(input))>>
			::iterator_category());
	}
	template<typename _tRange1, typename _tRange2>
	inline bool
		ends_with(const _tRange1& input, const _tRange2& test)
	{
		return leo::ends_with(input, test, is_equal());
	}
	//@}
	//@}

	/*!
	\brief 判断是否存在子串。
	\since build 1.4
	\todo 添加序列容器子串重载版本；优化：避免构造子串对象。
	*/
	//@{
	template<class _tString, typename _type>
	inline bool
		exists_substr(const _tString& str, const _type& sub)
	{
		return str.find(sub) != _tString::npos;
	}
	template<class _tString, typename _type>
	inline bool
		exists_substr(const _tString& str, const typename _tString::value_type* p_sub)
	{
		lconstraint(p_sub);

		return str.find(p_sub) != _tString::npos;
	}
	//@}


	//! \ingroup NTCTSUtil
	//@{
	/*!
	\brief NTCTS 正规化：保留空字符之前的字符。
	\post <tt>str.length() == leo::ntctslen(str.c_str())</tt> 。
	\since build 606
	*/
	template<class _tString>
	auto
		normalize(_tString& str) -> decltype(str.resize(leo::ntctslen(str.c_str())))
	{
		return str.resize(leo::ntctslen(str.c_str()));
	}

	/*!
	\brief 复制不超过指定长度的 NTCTS 。
	\note 目标字符串短于指定长度的部分可能被填充空字符。
	\warning 源字符串在指定长度内没有空字符则目标字符串不保证以空字符结尾。
	\since build 604
	*/
	//@{
	//! \pre 断言：指针参数非空。
	template<class _tString,
		limpl(typename = leo::enable_for_string_class_t<_tString>)>
		inline _tString&&
		ntctsncpy(_tString&& str, const typename string_traits<_tString>::const_pointer
			s, const typename string_traits<_tString>::size_type n)
	{
		lconstraint(s);
		return static_cast<_tString&&>(str = decay_t<_tString>(s, n));
	}
	template<class _tString,
		limpl(typename = leo::enable_for_string_class_t<_tString>)>
		inline _tString&&
		ntctsncpy(_tString&& str, _tString&& s,
			const typename string_traits<_tString>::size_type n)
	{
		auto sub(lforward(s));

		sub.resize(n);
		return static_cast<_tString&&>(str = std::move(sub));
	}
	//@}
	//@}


	/*!
	\brief 限制长度不超过指定参数。
	\since build 1.4
	*/
	template<class _tString>
	inline _tString&&
		restrict_length(_tString&& str,
			const typename string_traits<_tString>::size_type n)
	{
		if (n < str.length())
			str.resize(n);
		return static_cast<_tString&&>(str);
	}

	/*!
	\brief 取字母表：有序的字符串的不重复序列
	*/
	template<class _tString>
	_tString
		alph(_tString& str)
	{
		_tString res(str);

		leo::sort_unique(res);
		return res;
	}

	/*!
	\brief 重复串接。
	\param str 被串接的字符串的引用。
	\param n 串接结果包含原字符串的重复次数。
	\pre 断言： <tt>1 < n</tt> 。
	\todo 检查 reserve 。
	*/
	template<class _tString>
	void
		concat(_tString& str, size_t n)
	{
		lconstraint(n != 0);
		if (1 < n)
		{
			const auto len(str.length());

			//俄罗斯农民算法
			leo::concat(str, n / 2);
			str.append(str.data(), str.length());
			if (n % 2 != 0)
				str.append(str.data(), len);
		}
	}

	//!\pre 字符串类型满足 std::basic_string 或 basic_string_view 的操作及异常规范。
	//@{
	//! \since build 1.4
	//@{
	//! \brief 删除字符串中指定的连续字符左侧的字符串。
	//@{
	template<class _tString>
	inline limpl(enable_if_t) < is_class<decay_t<_tString>>::value, _tString&& >
		erase_left(typename string_traits<_tString>::size_type pos, _tString&& str)
	{
		return static_cast<_tString&&>(pos != decay_t<_tString>::npos
			? details::str_algo<>::erase_left(str, pos) : str);
	}
	template<class _tString>
	inline limpl(enable_if_t) < is_class<decay_t<_tString>>::value, _tString&& >
		erase_left(_tString&& str, typename string_traits<_tString>::value_type c)
	{
		return static_cast<_tString&&>(leo::erase_left(str.find_last_of(c), str));
	}
	template<class _tString>
	inline limpl(enable_if_t) < is_class<decay_t<_tString>>::value, _tString&& >
		erase_left(_tString&& str, const remove_reference_t<_tString>& t)
	{
		return static_cast<_tString&&>(leo::erase_left(str.find_last_of(t), str));
	}
	/*!
	\pre 断言：指针参数非空。
	\since build 1.4
	*/
	template<class _tString>
	inline limpl(enable_if_t) < is_class<decay_t<_tString>>::value, _tString&& >
		erase_left(_tString&& str, typename string_traits<_tString>::const_pointer t
			= &to_array<typename string_traits<_tString>::value_type>(" \f\n\r\t\v")[0])
	{
		lconstraint(t);
		return static_cast<_tString&&>(leo::erase_left(str.find_last_of(t), str));
	}
	//@}

	//! \brief 删除字符串中指定的连续字符右侧的字符串。
	//@{
	template<class _tString>
	inline limpl(enable_if_t) < is_class<decay_t<_tString>>::value, _tString&& >
		erase_right(typename string_traits<_tString>::size_type pos, _tString&& str)
	{
		return static_cast<_tString&&>(pos != decay_t<_tString>::npos
			? details::str_algo<>::erase_right(str, pos) : str);
	}
	template<class _tString>
	inline limpl(enable_if_t) < is_class<decay_t<_tString>>::value, _tString&& >
		erase_right(_tString&& str, typename string_traits<_tString>::value_type c)
	{
		return static_cast<_tString&&>(leo::erase_right(str.find_last_of(c),
			str));
	}
	//! \since build 1.4
	template<class _tString>
	inline limpl(enable_if_t) < is_class<decay_t<_tString>>::value, _tString&& >
		erase_right(_tString&& str, const remove_reference_t<_tString>& t)
	{
		return static_cast<_tString&&>(leo::erase_right(str.find_last_of(t),
			str));
	}
	/*!
	\pre 断言：指针参数非空。
	\since build 1.4
	*/
	template<class _tString>
	inline limpl(enable_if_t) < is_class<decay_t<_tString>>::value, _tString&& >
		erase_right(_tString&& str, typename string_traits<_tString>::const_pointer t
			= &to_array<typename string_traits<_tString>::value_type>(" \f\n\r\t\v")[0])
	{
		lconstraint(t);
		return static_cast<_tString&&>(leo::erase_right(str.find_last_of(t),
			str));
	}
	//@}
	//@}

	//! \since build 1.3
	//@{
	//! \brief 删除字符串中指定的连续前缀字符。
	//@{
	template<class _tString>
	lconstfn _tString&&
		ltrim(_tString&& str, typename string_traits<_tString>::value_type c)
	{
		return static_cast<_tString&&>(
			details::str_algo<>::erase_left(str, str.find_first_not_of(c)));
	}
	//! \since build 1.4
	template<class _tString>
	inline _tString&&
		ltrim(_tString&& str, const _tString& t)
	{
		return static_cast<_tString&&>(
			details::str_algo<>::erase_left(str, str.find_first_not_of(t)));
	}
	/*!
	\pre 断言：指针参数非空。
	\since build 1.4
	*/
	template<class _tString>
	lconstfn _tString&&
		ltrim(_tString&& str, typename string_traits<_tString>::const_pointer t
			= &to_array<typename string_traits<_tString>::value_type>(" \f\n\r\t\v")[0])
	{
		return lconstraint(t), static_cast<_tString&&>(
			details::str_algo<>::erase_left(str, str.find_first_not_of(t)));
	}
	//@}

	//! \brief 删除字符串中指定的连续后缀字符。
	//@{
	template<class _tString>
	lconstfn _tString&&
		rtrim(_tString&& str, typename string_traits<_tString>::value_type c)
	{
		return static_cast<_tString&&>(
			details::str_algo<>::erase_right(str, str.find_last_not_of(c)));
	}
	//! \since build 1.4
	template<class _tString>
	lconstfn _tString&&
		rtrim(_tString&& str, const remove_reference_t<_tString>& t) lnothrowv
	{
		return static_cast<_tString&&>(
			details::str_algo<>::erase_right(str, str.find_last_not_of(t)));
	}
	/*!
	\pre 断言：指针参数非空。
	\since build 1.4
	*/
	template<class _tString>
	lconstfn _tString&&
		rtrim(_tString&& str, typename string_traits<_tString>::const_pointer t
			= &to_array<typename string_traits<_tString>::value_type>(" \f\n\r\t\v")[0])
	{
		return lconstraint(t), static_cast<_tString&&>(
			details::str_algo<>::erase_right(str, str.find_last_not_of(t)));
	}
	//@}

	//! \brief 删除字符串中指定的连续前缀与后缀字符。
	//@{
	template<class _tString>
	lconstfn _tString&&
		trim(_tString&& str, typename string_traits<_tString>::value_type c)
	{
		return lforward(leo::ltrim(lforward(leo::rtrim(lforward(str), c)), c));
	}
	//! \since build 1.4
	template<class _tString>
	lconstfn _tString&&
		trim(_tString&& str, const _tString& t) lnothrowv
	{
		return lforward(leo::ltrim(lforward(leo::rtrim(lforward(str), t)), t));
	}
	/*!
	\pre 断言：指针参数非空。
	\since build 1.4
	*/
	template<class _tString>
	lconstfn _tString&&
		trim(_tString&& str, typename string_traits<_tString>::const_pointer t
			= &to_array<typename string_traits<_tString>::value_type>(" \f\n\r\t\v")[0])
	{
		return lconstraint(t),
			lforward(leo::ltrim(lforward(leo::rtrim(lforward(str), t)), t));
	}
	//@}
	//@}
	//@}


	//! \since build 1.4
	//@{
	//! \brief 取前缀子字符串。
	//@{
	template<typename _tString, typename... _tParams>
	inline _tString
		find_prefix(const _tString& str, _tParams&&... args)
	{
		if (!str.empty())
		{
			const auto pos(str.find(lforward(args)...));

			if (pos != _tString::npos)
				return str.substr(0, pos);
		}
		return{};
	}
	//@}

	//! \brief 取后缀子字符串。
	//@{
	template<typename _tString, typename... _tParams>
	inline _tString
		find_suffix(const _tString& str, _tParams&&... args)
	{
		if (!str.empty())
		{
			const auto pos(str.rfind(lforward(args)...));

			if (pos != _tString::npos)
				return str.substr(pos + 1);
		}
		return{};
	}
	//@}
	//@}

	/*!
	\brief 取相同的前缀和后缀元素。
	\pre 取前缀和后缀操作无副作用。
	\pre 字符串非空。
	\since build 1.4
	\todo 检查非成员 \c front 和 back \c 。
	\todo 支持前缀和后缀字符串。
	\todo 扩展到一般容器。
	*/
	//@{
	template<typename _tString>
	lconstfn typename string_traits<_tString>::value_type
		get_quote_mark_nonstrict(const _tString& str)
	{
		return str.front() == str.back() ? str.front()
			: typename string_traits<_tString>::value_type();
	}
	//@}

	/*!
	\brief 取添加前缀和后缀的字符串。
	\pre 断言：删除的字符串不大于串长。
	\since build 1.4
	*/
	//@{
	template<typename _tString>
	inline _tString
		quote(const _tString& str, typename string_traits<_tString>::value_type c
			= typename string_traits<_tString>::value_type('"'))
	{
		return c + str + c;
	}
	template<typename _tString, typename _tString2>
	inline limpl(enable_if_t)<!is_convertible<_tString2,
		typename string_traits<_tString>::value_type>::value, _tString>
		quote(const _tString& str, const _tString2& s)
	{
		return s + str + s;
	}
	template<typename _tString, typename _tPrefix, typename _tSuffix>
	inline _tString
		quote(const _tString& str, const _tPrefix& pfx, const _tSuffix& sfx)
	{
		return pfx + str + sfx;
	}
	//@}

	/*!
	\brief 取删除前缀和后缀的子字符串。
	\pre 断言：删除的字符串不大于串长。
	*/
	//@{
	template<typename _tString>
	inline _tString
		get_mid(const _tString& str, typename _tString::size_type l = 1)
	{
		lassume(!(str.size() < l * 2));
		return str.substr(l, str.size() - l * 2);
	}
	template<typename _tString>
	inline _tString
		get_mid(const _tString& str, typename _tString::size_type l,
			typename _tString::size_type r)
	{
		lassume(!(str.size() < l + r));
		return str.substr(l, str.size() - l - r);
	}
	//@}
	//@}


	/*!
	\brief 从输入流中取字符串。
	\since build 1.4
	*/
	//@{
	//! \note 同 std::getline ，除判断分隔符及附带的副作用由参数的函数对象决定。
	template<typename _tChar, class _tTraits, class _tAlloc, typename _func>
	std::basic_istream<_tChar, _tTraits>&
		extract(std::basic_istream<_tChar, _tTraits>& is,
			std::basic_string<_tChar, _tTraits, _tAlloc>& str, _func f)
	{
		std::string::size_type n(0);
		auto st(std::ios_base::goodbit);

		if (const auto k
			= typename std::basic_istream<_tChar, _tTraits>::sentry(is, true))
		{
			const auto msize(str.max_size());
			const auto p_buf(is.rdbuf());

			lassume(p_buf);
			str.erase();
			try
			{
				const auto eof(_tTraits::eof());

				for (auto c(p_buf->sgetc()); ; c = p_buf->snextc())
				{
					if (_tTraits::eq_int_type(c, eof))
					{
						st |= std::ios_base::eofbit;
						break;
					}
					if (f(c, *p_buf))
					{
						++n;
						break;
					}
					if (!(str.length() < msize))
					{
						st |= std::ios_base::failbit;
						break;
					}
					str.append(1, _tTraits::to_char_type(c));
					++n;
				}
			}
			catch (...)
			{
				// NOTE: See http://wg21.cmeerw.net/lwg/issue91.
				rethrow_badstate(is, std::ios_base::badbit);
			}
		}
		if (n == 0)
			st |= std::ios_base::failbit;
		if (st)
			is.setstate(st);
		return is;
	}

	//! \note 同 std::getline ，除字符串结尾包含分隔符。
	//@{
	template<typename _tChar, class _tTraits, class _tAlloc>
	std::basic_istream<_tChar, _tTraits>&
		extract_line(std::basic_istream<_tChar, _tTraits>& is,
			std::basic_string<_tChar, _tTraits, _tAlloc>& str, _tChar delim)
	{
		const auto d(_tTraits::to_int_type(delim));

		return leo::extract(is, str,
			[d, &str](typename std::basic_istream<_tChar, _tTraits>::int_type c,
				std::basic_streambuf<_tChar, _tTraits>& sb) -> bool {
			if (_tTraits::eq_int_type(c, d))
			{
				sb.sbumpc();
				// NOTE: If not appended here, this function template shall be
				//	equivalent with %std::getline.
				str.append(1, d);
				return true;
			}
			return{};
		});
	}
	template<typename _tChar, class _tTraits, class _tAlloc>
	inline std::basic_istream<_tChar, _tTraits>&
		extract_line(std::basic_istream<_tChar, _tTraits>& is,
			std::basic_string<_tChar, _tTraits, _tAlloc>& str)
	{
		return leo::extract_line(is, str, is.widen('\n'));
	}

	/*!
	\brief 同 \c leo::extract_line ，但允许分隔符包含附加的前缀字符。
	\note 默认 \c LF 作为基准分隔符，前缀为 \c CR ，即接受 \c LF 和 <tt>CR+LF</tt> 。
	\note 一般配合二进制方式打开的流使用，以避免不必要的分隔符转换。
	*/
	template<typename _tChar, class _tTraits, class _tAlloc>
	std::basic_istream<_tChar, _tTraits>&
		extract_line_cr(std::basic_istream<_tChar, _tTraits>& is,
			std::basic_string<_tChar, _tTraits, _tAlloc>& str, _tChar delim_cr = '\r',
			_tChar delim = '\n')
	{
		const auto cr(_tTraits::to_int_type(delim_cr));
		const auto d(_tTraits::to_int_type(delim));

		return leo::extract(is, str,
			[cr, d, &str](typename std::basic_istream<_tChar, _tTraits>::int_type c,
				std::basic_streambuf<_tChar, _tTraits>& sb) -> bool {
			if (_tTraits::eq_int_type(c, d))
				str.append(1, d);
			else if (_tTraits::eq_int_type(c, cr)
				&& _tTraits::eq_int_type(sb.sgetc(), d))
			{
				sb.sbumpc();
				str.append(1, cr);
				str.append(1, d);
			}
			else
				return{};
			sb.sbumpc();
			return true;
		});
	}
	//@}
	//@}

	/*!
	\brief 非格式输出。
	\since build 1.4
	*/
	//@{
	template<typename _tChar, class _tString>
	std::basic_ostream<_tChar, typename _tString::traits_type>&
		write(std::basic_ostream<_tChar, typename _tString::traits_type>& os,
			const _tString& str, typename _tString::size_type pos = 0,
			typename _tString::size_type n = _tString::npos)
	{
		const auto len(str.length());

		if (pos < len)
			os.write(&str[pos], std::streamsize(std::min(n, len - pos)));
		return os;
	}
	//! \since build 1.4
	template<typename _tChar, class _tTraits, size_t _vN>
	std::basic_ostream<_tChar, _tTraits>&
		write(std::basic_ostream<_tChar, _tTraits>& os, const _tChar(&s)[_vN])
	{
		return os.write(std::addressof(s[0]), std::streamsize(size(s)));
	}

	/*!
	\note 参数数组作为字符串字面量。
	\since build 1.4
	*/
	template<typename _tChar, class _tTraits, size_t _vN>
	std::basic_ostream<_tChar, _tTraits>&
		write_literal(std::basic_ostream<_tChar, _tTraits>& os, const _tChar(&s)[_vN])
	{
		static_assert(0 < _vN, "Empty string literal found.");

		return
			os.write(std::addressof(s[0]), std::streamsize(size(s) - 1));
	}
	//@}

	/*!
	\brief 转换为字符串： std::basic_string 的实例对象。
	\note 可与标准库的同名函数共用以避免某些类型转换警告，如 G++ 的 [-Wsign-promo] 。
	*/
	//@{
	//! \since build 1.3
	//@{
	inline std::string
		to_string(unsigned char val)
	{
		return std::to_string(unsigned(val));
	}	
	inline std::string
		to_string(unsigned short val)
	{
		return std::to_string(unsigned(val));
	}
	template<typename _type>
	inline limpl(enable_if_t)<is_enum<_type>::value,std::string>
		to_string(_type val)
	{
		using std::to_string;
		using leo::to_string;

		return to_string(underlying(val));
	}
	template<typename _type, class _tStream = std::ostringstream>
	limpl(enable_if_t)<is_class<_type>::value,
		decltype(std::declval<_tStream&>().str())>
		to_string(const _type& x)
	{
		_tStream oss;

		oss << x;
		return oss.str();
	}
	inline std::string
		to_string(const std::string& str)
	{
		return str;
	}
	//@}
	

	//! \since build 1.4
	//@{
	inline std::wstring
		to_wstring(unsigned char val)
	{
		return std::to_wstring(unsigned(val));
	}
	inline std::wstring
		to_wstring(unsigned short val)
	{
		return std::to_wstring(unsigned(val));
	}
	//! \since build 1.4
	template<typename _type>
	inline limpl(enable_if_t)<is_enum<_type>::value, std::wstring>
		to_wstring(_type val)
	{
		using std::to_wstring;
		using leo::to_wstring;

		return to_wstring(leo::underlying(val));
	}
	template<typename _type, class _tStream = std::wostringstream>
	limpl(enable_if_t)<is_class<_type>::value,
		decltype(std::declval<_tStream&>().str())>
		to_wstring(const _type& x)
	{
		_tStream oss;

		oss << x;
		return oss.str();
	}
	inline std::wstring
		to_wstring(const std::wstring& str)
	{
		return str;
	}
	//@}
	//@}

	namespace details
	{

		template<typename _tString, typename _type>
		struct ston_dispatcher;

#define LB_Impl_String_ston_begin(_tString, _type, _n, ...) \
	template<> \
	struct ston_dispatcher<_tString, _type> \
	{ \
		static _type \
		cast(const _tString& str, __VA_ARGS__) \
		{ \
			return _n(str
#define LB_Impl_String_ston_end \
			); \
		} \
	};

#define LB_Impl_String_ston_i(_tString, _type, _n) \
	LB_Impl_String_ston_begin(_tString, _type, _n, size_t* idx = {}, \
		int base = 10), idx, base \
	LB_Impl_String_ston_end
#define LB_Impl_String_ston_i_std(_type, _n) \
	LB_Impl_String_ston_i(std::string, _type, std::_n)
		LB_Impl_String_ston_i_std(int, stoi)
			LB_Impl_String_ston_i_std(long, stol)
			LB_Impl_String_ston_i_std(unsigned long, stoul)
			LB_Impl_String_ston_i_std(long long, stoll)
			LB_Impl_String_ston_i_std(unsigned long long, stoull)
#undef LB_Impl_String_ston_i_std
#undef LB_Impl_String_ston_i

#define LB_Impl_String_ston_f(_tString, _type, _n) \
	LB_Impl_String_ston_begin(_tString, _type, _n, size_t* idx = {}), idx \
	LB_Impl_String_ston_end
#define LB_Impl_String_ston_f_std(_type, _n) \
	LB_Impl_String_ston_f(std::string, _type, std::_n)
#	ifndef __BIONIC__
			LB_Impl_String_ston_f_std(float, stof)
			LB_Impl_String_ston_f_std(double, stod)
			LB_Impl_String_ston_f_std(long double, stold)
#	endif
#undef LB_Impl_String_ston_f_std
#undef LB_Impl_String_ston_f

#undef LB_Impl_String_ston_end
#undef LB_Impl_String_ston_begin

	} // namespace details;


	/*!
	\brief 转换表示数的字符串。
	\since build 1.4
	\todo 支持 std::string 以外类型字符串。
	\todo 支持标准库以外的转换。
	*/
	template<typename _type, typename _tString, typename... _tParams>
	inline _type
		ston(const _tString& str, _tParams&&... args)
	{
		return details::ston_dispatcher<decay_t<_tString>, _type>::cast(str,
			lforward(args)...);
	}


	/*!
	\pre 间接断言：第一参数非空。
	\since build 1.4
	\bug \c char 以外的模板参数非正确实现。
	*/
	//@{
	/*!
	\brief 以 C 标准输出格式的输出 std::basic_string 实例的对象。
	\throw std::runtime_error 格式化字符串输出失败。
	\note 对 _tString 构造异常中立。
	*/
	template<typename _tChar, class _tString = std::basic_string<_tChar>>
	LB_NONNULL(1) _tString
		vsfmt(const _tChar* fmt, std::va_list args)
	{
		std::va_list ap;

		va_copy(ap, args);

		const auto l(leo::vfmtlen(fmt, ap));

		va_end(ap);
		if (l == size_t(-1))
			throw std::runtime_error("Failed to write formatted string.");

		_tString str(l, _tChar());

		if (l != 0)
		{
			lassume(str.length() > 0 && str[0] == _tChar());
			std::vsprintf(&str[0], fmt, args);
		}
		return str;
	}
	
	/*!
	\brief 以 C 标准输出格式的输出 std::basic_string 实例的对象。
	\note 使用 ADL 访问可变参数。
	\note Clang++ 对模板声明 attribute 直接提示格式字符串类型错误。
	*/
	template<typename _tChar>
	LB_NONNULL(1) auto
		sfmt(const _tChar* fmt, ...)
		-> decltype(vsfmt(fmt, std::declval<std::va_list>()))
	{
		std::va_list args;

		va_start(args, fmt);
		try
		{
			auto str(vsfmt(fmt, args));

			va_end(args);
			return str;
		}
		catch (...)
		{
			va_end(args);
			throw;
		}
	}
	//@}

#if LB_IMPL_MSCPP
#else
	template LB_ATTR(format(printf, 1, 2)) std::string
		sfmt<char>(const char*, ...);
#endif

	/*!
	\ingroup string_algorithms
	\brief 过滤前缀：存在前缀时处理移除前缀后的子串。
	\since build 1.4
	*/
	template<typename _type, typename _tString, typename _func>
	bool
		filter_prefix(const _tString& str, const _type& prefix, _func f)
	{
		if (leo::begins_with(str, prefix))
		{
			auto&& sub(str.substr(string_length(prefix)));

			if (!sub.empty())
				f(std::move(sub));
			return true;
		}
		return{};
	}

	/*!
	\brief 选择性过滤前缀。
	\return 指定前缀存在时为去除前缀的部分，否则为参数指定的替代值。
	\since build 1.4
	*/
	template<typename _type, typename _tString>
	_tString
		cond_prefix(const _tString& str, const _type& prefix, _tString&& val = {})
	{
		leo::filter_prefix(str, prefix, [&](_tString&& s)
			lnoexcept(is_nothrow_move_assignable<decay_t<_tString>>::value) {
			val = std::move(s);
		});
		return std::move(val);
	}

	template<typename _tString,typename _tChar =typename string_traits<_tString>::value_type>
	void to_lower(_tString&& str) {
		std::transform(string_begin(str),string_end(str),string_begin(str), [](_tChar c)
		{
			return static_cast<_tChar>(std::tolower(c));
		}
		);
	}
}
#endif