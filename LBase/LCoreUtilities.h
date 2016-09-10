/*!	\file LCoreUtilities.h
\ingroup Core
\brief ����ʵ��ģ�顣
*/

#ifndef LEO_Core_LCoreUtilities_h
#define LEO_Core_LCoreUtilities_h 1

#include <LBase/LException.h>
#include <LBase/algorithm.hpp>

namespace leo {
	/*!
	\throw LoggedEvent ��Χ���ʧ�ܡ�
	*/
	//@{
	//! \brief ��鴿����ֵ��ָ�����͵ķ�Χ�ڡ�
	template<typename _tDst, typename _type>
	inline _tDst
		CheckScalar(_type val, const std::string& name = {}, RecordLevel lv = Err)
	{
		using common_t = common_type_t<_tDst, _type>;

		if (LB_UNLIKELY(common_t(val) > common_t(std::numeric_limits<_tDst>::max())))
			throw LoggedEvent(name + " value out of range.", lv);
		return _tDst(val);
	}

	//! \brief ���Ǹ�������ֵ��ָ�����͵ķ�Χ�ڡ�
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

	//! \brief �����������ֵ��ָ�����͵ķ�Χ�ڡ�
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