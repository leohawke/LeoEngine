/*!	\file LCoreUtilities.h
\ingroup LFrameWork/Core
\brief 核心实用模块。
*/

#ifndef LFrameWork_Core_LCoreUtilities_h
#define LFrameWork_Core_LCoreUtilities_h 1

#include <LBase/algorithm.hpp>
#include <LFramework/Core/LException.h>

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

	template<typename _tDst, typename _type>
	inline _tDst
		CheckLowerBound(_type val, const std::string& name = {}, RecordLevel lv = Err)
	{
		using namespace leo;
		// XXX: See WG21 N3387.
		// TODO: Add and use safe %common_arithmetic_type interface instead?
		using common_t = typename leo::common_int_type<_tDst, _type>::type;

		if (!(common_t(val) < common_t(std::numeric_limits<_tDst>::min())))
			return _tDst(val);
		throw
			LoggedEvent("Value of '" + name + "' is less than lower boundary.", lv);
	}

	//! \brief 检查算术类型数值不大于指定类型的上界。
	template<typename _tDst, typename _type>
	inline _tDst
		CheckUpperBound(_type val, const std::string& name = {}, RecordLevel lv = Err)
	{
		using namespace leo;
		// XXX: See WG21 N3387.
		// TODO: Add and use safe %common_arithmetic_type interface instead?
		using common_t = typename leo::common_int_type<_tDst, _type>::type;

		if ((std::is_signed<common_t>() && std::is_unsigned<_tDst>()
			&& leo::integer_width<common_t>() <= leo::integer_width<_tDst>())
			|| !(common_t(std::numeric_limits<_tDst>::max()) < common_t(val)))
			return _tDst(val);
		throw LoggedEvent("Value of '" + name + "' is greater than upper boundary.",
			lv);
	}

	//! \brief 检查算术类型数值在指定类型的范围内。
	template<typename _tDst, typename _type>
	inline _tDst
		CheckArithmetic(_type val, const std::string& name = {}, RecordLevel lv = Err)
	{
		return
			CheckUpperBound<_tDst>(CheckLowerBound<_tDst>(val, name, lv), name, lv);
	}


	//! \brief 检查非负算术类型数值在指定类型的范围内。
	template<typename _tDst, typename _type>
	inline _tDst
		CheckNonnegative(_type val, const std::string& name = {}, RecordLevel lv = Err)
	{
		if (val < 0)
			// XXX: Use more specified exception type.
			throw LoggedEvent("Failed getting nonnegative " + name + " value.", lv);
		return CheckUpperBound<_tDst>(val, name, lv);
	}
	template<typename _tDst, typename _type>
	inline _tDst
		CheckPositive(_type val, const std::string& name = {}, RecordLevel lv = Err)
	{
		if (!(0 < val))
			// XXX: Use more specified exception type.
			throw LoggedEvent("Failed getting positive " + name + " value.", lv);
		return CheckUpperBound<_tDst>(val, name, lv);
	}

	/*!
	\brief 参数字符串向量。

	用于存储命令行参数等字符串向量。
	*/
	class LF_API ArgumentsVector final
	{
	public:
		vector<string> Arguments{};

		/*!
		\brief 设置值。
		\pre 间接断言： 第二参数的元素都是非空指针。
		\exception LoggedEvent 输入的参数数小于 0 。
		\sa CheckNonnegative

		设置参数向量的值。
		参数分别指定参数向量和表示 NTMBS 的指针数组。
		*/
		void
			Reset(int, char*[]);
	};


	/*!
	\brief 查询第二参数指定名称的环境变量写入第一参数。
	\pre 断言：参数非空。
	\note 若不存在则不修改第一参数。
	\return 是否修改第一参数。
	*/
	LF_API LB_NONNULL(2) bool
		FetchEnvironmentVariable(string&, const char*);
}
#endif
