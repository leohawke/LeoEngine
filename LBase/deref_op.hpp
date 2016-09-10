/*! \file deref_op.hpp
\ingroup LBase
\brief 解引用操作。
*/

#ifndef LBase_deref_op_hpp
#define LBase_deref_op_hpp 1

#include "LBase/ldef.h"

namespace leo {
	/*!	\defgroup is_undereferenceable Is Undereferenceable Iterator
	\brief 判断迭代器实例是否不可解引用。
	\tparam _tIter 迭代器类型。
	\note 注意返回 \c false 不表示参数实际可解引用。
	*/
	//@{
	template<typename _tIter>
	lconstfn bool
		is_undereferenceable(const _tIter&) lnothrow
	{
		return{};
	}
	template<typename _type>
	lconstfn bool
		is_undereferenceable(_type* p) lnothrow
	{
		return !p;
	}
	//@}


	//! \since build 1.4
	//@{
	//! \brief 取非空引用或默认值。
	//@{
	template<typename _tOther, typename _type>
	lconstfn auto
		nonnull_or(_type p, _tOther&& other = {}) -> decltype(p ? p : other)
	{
		return p ? p : other;
	}
	template<typename _tOther, typename _type, typename _tNull = nullptr_t>
	lconstfn auto
		nonnull_or(_type p, _tOther&& other, _tNull null)
		-> decltype(!bool(p == null) ? p : other)
	{
		return !bool(p == null) ? p : other;
	}
	//@}


	//! \brief 取非空值或默认值。
	//@{
	template<typename _tOther, typename _type>
	lconstfn auto
		value_or(_type p, _tOther&& other = {}) -> decltype(p ? *p : other)
	{
		return p ? *p : other;
	}
	template<typename _tOther, typename _type, typename _tSentinal = nullptr_t>
	lconstfn auto
		value_or(_type p, _tOther&& other, _tSentinal last)
		-> decltype(!bool(p == last) ? *p : other)
	{
		return !bool(p == last) ? *p : other;
	}
	//@}


	//! \brief 调用非空值或取默认值。
	//@{
	template<typename _tOther, typename _func, typename _type>
	lconstfn auto
		call_value_or(_func f, _type p, _tOther&& other = {})
		-> decltype(p ? f(*p) : other)
	{
		return p ? f(*p) : other;
	}
	template<typename _tOther, typename _func, typename _type,
		typename _tSentinal = nullptr_t>
		lconstfn auto
		call_value_or(_func f, _type p, _tOther&& other, _tSentinal last)
		-> decltype(!bool(p == last) ? f(*p) : other)
	{
		return !bool(p == last) ? f(*p) : other;
	}
	//@}
	//@}
}

#endif