/*! \file deref_op.hpp
\ingroup LBase
\brief 解引用操作。
\par 修改时间:
2016-12-10 01:51 +0800
*/

#ifndef LBase_deref_op_hpp
#define LBase_deref_op_hpp 1

#include "LBase/type_traits.hpp"

using stdex::nullptr_t;

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
	template<typename _type>
	lconstfn auto
		nonnull_or(_type&& p) -> decay_t<decltype(lforward(p))>
	{
		return p ? lforward(p) : decay_t<decltype(lforward(p))>();
	}
	template<typename _tOther, typename _type>
	lconstfn auto
		nonnull_or(_type&& p, _tOther&& other)
		->limpl(decltype(p ? lforward(p) : lforward(other)))
	{
		return p ? lforward(p) : lforward(other);
	}
	template<typename _tOther, typename _type, typename _tSentinal = nullptr_t>
	lconstfn auto
		nonnull_or(_type&& p, _tOther&& other, _tSentinal&& last)->limpl(
			decltype(!bool(p == lforward(last)) ? lforward(p) : lforward(other)))
	{
		return !bool(p == lforward(last)) ? lforward(p) : lforward(other);
	}
	//@}

	/*!
	\brief 调用非引用或默认值。
	*/
	//@{
	template<typename _type, typename _func>
	lconstfn auto
		call_nonnull_or(_func f, _type&& p) -> decay_t<decltype(f(lforward(p)))>
	{
		return p ? f(lforward(p)) : decay_t<decltype(f(lforward(p)))>();
	}
	template<typename _tOther, typename _type, typename _func>
	lconstfn auto
		call_nonnull_or(_func f, _type&& p, _tOther&& other)
		->limpl(decltype(p ? f(lforward(p)) : lforward(other)))
	{
		return p ? f(lforward(p)) : lforward(other);
	}
	template<typename _tOther, typename _type, typename _func,
		typename _tSentinal = nullptr_t>
		lconstfn auto
		call_nonnull_or(_func f, _type&& p, _tOther&& other, _tSentinal&& last)
		->limpl(
			decltype(!bool(p == lforward(last)) ? f(lforward(p)) : lforward(other)))
	{
		return !bool(p == lforward(last)) ? f(lforward(p)) : lforward(other);
	}
	//@}

	//! \brief 取非空值或默认值。
	//@{
	template<typename _type>
	lconstfn auto
		value_or(_type&& p) -> decay_t<decltype(*lforward(p))>
	{
		return p ? *lforward(p) : decay_t<decltype(*lforward(p))>();
	}
	template<typename _tOther, typename _type>
	lconstfn auto
		value_or(_type&& p, _tOther&& other)
		->limpl(decltype(p ? *lforward(p) : lforward(other)))
	{
		return p ? *lforward(p) : lforward(other);
	}
	template<typename _tOther, typename _type, typename _tSentinal = nullptr_t>
	lconstfn auto
		value_or(_type&& p, _tOther&& other, _tSentinal&& last)->limpl(
			decltype(!bool(p == lforward(last)) ? *lforward(p) : lforward(other)))
	{
		return !bool(p == lforward(last)) ? *lforward(p) : lforward(other);
	}
	//@}


	//! \brief 调用非空值或取默认值。
	//@{
	template<typename _type, typename _func>
	lconstfn auto
		call_value_or(_func f, _type&& p) -> decay_t<decltype(f(*lforward(p)))>
	{
		return p ? f(*lforward(p)) : decay_t<decltype(f(*lforward(p)))>();
	}
	template<typename _tOther, typename _type, typename _func>
	lconstfn auto
		call_value_or(_func f, _type&& p, _tOther&& other)
		->limpl(decltype(p ? f(*lforward(p)) : lforward(other)))
	{
		return p ? f(*lforward(p)) : lforward(other);
	}
	template<typename _tOther, typename _type, typename _func,
		typename _tSentinal = nullptr_t>
		lconstfn auto
		call_value_or(_func f, _type&& p, _tOther&& other, _tSentinal&& last)->limpl(
			decltype(!bool(p == lforward(last)) ? f(*lforward(p)) : lforward(other)))
	{
		return !bool(p == lforward(last)) ? f(*lforward(p)) : lforward(other);
	}
	//@}
	//@}
}

#endif