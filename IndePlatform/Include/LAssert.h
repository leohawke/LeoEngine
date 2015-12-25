#ifndef IndePlatform_LAssert_h
#define IndePlatform_LAssert_h

#include "deref_op.hpp"
#include <assert.h>

#pragma warning(push)
#pragma warning(disable:4800)

#undef LAssert
#define LAssert(_expr,_msg) assert(_expr)

#ifndef LAssertNonnull
#define LAssertNonnull(_expr) LAssert(bool(_expr), "Null reference found.")
#endif

//不满足此断言的行为是接口明确地未定义的，行为不可预测
#define lconstraint assert
//运行时检查的环境条件约束断言。用于明确地非 yconstraint 适用的情形
#define lassume assert

namespace leo {

	/*!
	\brief 断言并返回非空指针。
	\pre 断言：指针非空。
	*/
	template<typename _type>
	inline _type&&
		Nonnull(_type&& p) lnothrow
	{
		LAssertNonnull(p);
		return lforward(p);
	}

	/*!
	\brief 检查迭代器。
	\pre 断言：迭代器非确定不可解引用。
	*/
	template<typename _type>
	inline _type&&
		CheckIter(_type&& i) lnothrow
	{
		LAssert(!is_undereferenceable(i), "Invalid iterator found.");
		return lforward(i);
	}


	/*!
	\brief 断言并解引用非空指针。
	\pre 使用 ADL 指定的 CheckIter 调用的表达式语义等价于 platform::CheckIter 。
	\pre 间接断言：指针非空。
	*/
	template<typename _type>
	lconstfn auto
		Deref(_type&& p) -> decltype(*p)
	{
		return *CheckIter(lforward(p));
	}
}

#pragma warning(pop)
#endif
