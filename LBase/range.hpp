/*!	\file iterator_op.hpp
\ingroup LBase
\brief ·¶Î§²Ù×÷¡£
*/

#ifndef LBase_range_hpp
#define LBase_range_hpp 1

#include "LBase/ldef.h"
#include <iterator> // for std::begin,std::end

namespace leo
{

	//! \since build 1.4
	//@{
	inline namespace cpp2011
	{

		using std::begin;
		//! \since build 1.4
		template<typename _type, size_t _vN>
		lconstfn _type*
			begin(_type(&&array)[_vN]) lnothrow
		{
			return array;
		}
		using std::end;
		//! \since build 1.4
		template<typename _type, size_t _vN>
		lconstfn _type*
			end(_type(&&array)[_vN]) lnothrow
		{
			return array + _vN;
		}

	} // inline namespace cpp2011;

	inline namespace cpp2014
	{

#if (__cplusplus >= 201402L && (!defined(__GLIBCXX__) \
	|| __GLIBCXX__ >= 20150119)) || (_LIBCXX_VERSION >= 1101 \
	&& _LIBCPP_STD_VER > 11) || LB_IMPL_MSCPP >= 1800
		using std::cbegin;
		using std::cend;
		using std::rbegin;
		using std::rend;
		using std::crbegin;
		using std::crend;
#else
		// TODO: 'constexpr' on 'std::begin' and 'std::end' for array types since
		//	C++14?

		template<typename _tRange>
		lconstfn auto
			cbegin(const _tRange& c) lnoexcept_spec(std::begin(c))
			-> decltype(std::begin(c))
		{
			return std::begin(c);
		}

		template<typename _tRange>
		lconstfn auto
			cend(const _tRange& c) lnoexcept_spec(std::end(c)) -> decltype(std::end(c))
		{
			return std::end(c);
		}

		template<class _tRange>
		auto
			rbegin(_tRange& c) -> decltype(c.rbegin())
		{
			return c.rbegin();
		}
		template<class _tRange>
		auto
			rbegin(const _tRange& c) -> decltype(c.rbegin())
		{
			return c.rbegin();
		}
		template<typename _type, size_t _vN>
		std::reverse_iterator<_type*>
			rbegin(_type(&array)[_vN])
		{
			return std::reverse_iterator<_type*>(array + _vN);
		}

		template<typename _tElem>
		std::reverse_iterator<const _tElem*>
			rbegin(std::initializer_list<_tElem> il)
		{
			return std::reverse_iterator<const _tElem*>(il.end());
		}

		template<class _tRange>
		auto rend(_tRange& c) -> decltype(c.rend())
		{
			return c.rend();
		}
		template<class _tRange>
		auto rend(const _tRange& c) -> decltype(c.rend())
		{
			return c.rend();
		}
		template<typename _type, size_t _vN>
		std::reverse_iterator<_type*>
			rend(_type(&array)[_vN])
		{
			return std::reverse_iterator<_type*>(array);
		}
		template<typename _tElem>
		std::reverse_iterator<const _tElem*>
			rend(std::initializer_list<_tElem> il)
		{
			return std::reverse_iterator<const _tElem*>(il.begin());
		}

		template<typename _tRange>
		auto
			crbegin(const _tRange& c) -> decltype(leo::rbegin(c))
		{
			return leo::rbegin(c);
		}

		template<typename _tRange>
		auto
			crend(const _tRange& c) -> decltype(leo::rend(c))
		{
			return leo::rend(c);
		}
#endif

		//! \since build 1.4
		//@{
		template<typename _type, size_t _vN>
		lconstfn _type*
			cbegin(_type(&&array)[_vN]) lnothrow
		{
			return array;
		}

		template<typename _type, size_t _vN>
		lconstfn _type*
			cend(_type(&&array)[_vN]) lnothrow
		{
			return array + _vN;
		}

		template<typename _type, size_t _vN>
		std::reverse_iterator<_type*>
			rbegin(_type(&&array)[_vN])
		{
			return std::reverse_iterator<_type*>(array + _vN);
		}

		template<typename _type, size_t _vN>
		std::reverse_iterator<_type*>
			rend(_type(&&array)[_vN])
		{
			return std::reverse_iterator<_type*>(array);
		}

#if __cplusplus <= 201402L
		//! \see http://wg21.cmeerw.net/cwg/issue1591 ¡£
		//@{
		template<typename _tElem>
		lconstfn const _tElem*
			cbegin(std::initializer_list<_tElem> il) lnothrow
		{
			return il.begin();
		}

		//! \see http://wg21.cmeerw.net/cwg/issue1591 ¡£
		template<typename _tElem>
		lconstfn const _tElem*
			cend(std::initializer_list<_tElem> il) lnothrow
		{
			return il.end();
		}

		template<typename _tElem>
		lconstfn std::reverse_iterator<const _tElem*>
			crbegin(std::initializer_list<_tElem> il) lnothrow
		{
			return leo::rbegin(il);
		}

		template<typename _tElem>
		lconstfn std::reverse_iterator<const _tElem*>
			crend(std::initializer_list<_tElem> il) lnothrow
		{
			return leo::rend(il);
		}
		//@}
#endif
		//@}

	} // inline namespace cpp2014;
	  //@}

	  /*!
	  \brief ÀàÈÝÆ÷·ÃÎÊ¡£
	  \see WG21/N4280 ¡£
	  \see WG21/N4567 24.8[iterator.container] ¡£
	  \since build 1.4
	  */
	  //@{
	template<class _tRange>
	lconstfn auto
		size(const _tRange& c) -> decltype(c.size())
	{
		return c.size();
	}
	template<typename _type, std::uint32_t _vN>
	lconstfn  std::uint32_t
		size(const _type(&)[_vN]) lnothrow
	{
		return _vN;
	}
#if __cplusplus <= 201402L
	//! \see http://wg21.cmeerw.net/cwg/issue1591 ¡£
	template<typename _tElem>
	lconstfn size_t
		size(std::initializer_list<_tElem> il) lnothrow
	{
		return il.size();
	}
#endif

	template<class _tRange>
	lconstfn auto
		empty(const _tRange& c) -> decltype(c.empty())
	{
		return c.empty();
	}
	template<typename _type, size_t _vN>
	lconstfn bool
		empty(const _type(&)[_vN]) lnothrow
	{
		return{};
	}
	template<typename _type, size_t _vN>
	lconstfn bool
		empty(const _type(&&)[_vN]) lnothrow
	{
		return{};
	}
	template<typename _tElem>
	lconstfn bool
		empty(std::initializer_list<_tElem> il) lnothrow
	{
		return il.size() == 0;
	}

	template<typename _tRange>
	lconstfn auto
		data(_tRange& c) -> decltype(c.data())
	{
		return c.data();
	}
	template<typename _tRange>
	lconstfn auto
		data(const _tRange& c) -> decltype(c.data())
	{
		return c.data();
	}
	template<typename _type, size_t _vN>
	lconstfn _type*
		data(_type(&array)[_vN]) lnothrow
	{
		return array;
	}
	template<typename _type, size_t _vN>
	lconstfn _type*
		data(_type(&&array)[_vN]) lnothrow
	{
		return array;
	}
	template<typename _tElem>
	lconstfn const _tElem*
		data(std::initializer_list<_tElem> il) lnothrow
	{
		return il.begin();
	}
	//@}

} // namespace leo;

#endif
