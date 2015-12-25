#ifndef IndePlatform_deref_op_hpp
#define IndePlatform_deref_op_hpp


#include "ldef.h"

namespace std
{
	template<typename _type>
	struct atomic;
}

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
	template<typename _type>
	lconstfn bool
		is_undereferenceable(const std::atomic<_type>&) = delete;
	template<typename _type>
	lconstfn bool
		is_undereferenceable(const volatile std::atomic<_type>&) = delete;
	//@}
}

#endif