/*!	\file LCoreUtilities.h
\ingroup Core
\brief 核心实用模块。
*/

#ifndef FrameWork__LCoreUtilities_h
#define FrameWork__LCoreUtilities_h 1

#include <LFramework/Core/LException.h>
#include <LBase/algorithm.hpp>

namespace leo {
	/*!
	\throw LoggedEvent 范围检查失败。
	*/
	//@{
	//! \brief 检查纯量数值在指定类型的范围内。
	template<typename _tDst, typename _type>
	inline _tDst
		CheckScalar(_type val, const std::string& name = {}, RecordLevel lv = Err)
	{
		using common_t = common_type_t<_tDst, _type>;

		if (LB_UNLIKELY(common_t(val) > common_t(std::numeric_limits<_tDst>::max())))
			throw LoggedEvent(name + " value out of range.", lv);
		return _tDst(val);
	}

	//! \brief 检查非负纯量数值在指定类型的范围内。
	template<typename _tDst, typename _type>
	inline _tDst
		CheckNonnegativeScalar(_type val, const std::string& name = {},
			RecordLevel lv = Err)
	{
		if (val < 0)
			// XXX: Use more specified exception type.
			throw LoggedEvent("Failed getting nonnegative " + name + " value.", lv);
		return CheckScalar<_tDst>(val, name, lv);
	}

	//! \brief 检查正纯量数值在指定类型的范围内。
	template<typename _tDst, typename _type>
	inline _tDst
		CheckPositiveScalar(_type val, const std::string& name = {},
			RecordLevel lv = Err)
	{
		if (!(0 < val))
			// XXX: Use more specified exception type.
			throw LoggedEvent("Failed getting positive " + name + " value.", lv);
		return CheckScalar<_tDst>(val, name, lv);
	}

	/*!
	\brief 判断 i 是否在闭区间 [_type(0), b] 中。
	\pre 断言： <tt>_type(0) < b</tt> 。
	*/
	template<typename _type>
	inline LB_STATELESS bool
		IsInClosedInterval(_type i, _type b) lnothrow
	{
		LAssert(_type(0) < b,
			"Zero element as lower bound is not less than upper bound.");
		return !(i < _type(0) || b < i);
	}
	/*!
	\brief 判断 i 是否在闭区间 [a, b] 中。
	\pre 断言： <tt>a < b</tt> 。
	*/
	template<typename _type>
	inline LB_STATELESS bool
		IsInClosedInterval(_type i, _type a, _type b) lnothrow
	{
		LAssert(a < b, "Lower bound is not less than upper bound.");
		return !(i < a || b < i);
	}

	//@}
}
#endif
