/*!	\file LCoreUtilities.h
\ingroup Core
\brief 核心实用模块。
*/

#ifndef FrameWork__LCoreUtilities_h
#define FrameWork__LCoreUtilities_h 1

#include <LBase/LException.h>
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
	//@}
}
#endif
