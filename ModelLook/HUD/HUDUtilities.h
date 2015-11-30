#include <ldef.h>
#include <LAssert.h>

namespace  leo
{

	namespace HUD {

		/*!
		\brief 判断 i 是否在左闭右开区间 [_type(0), b) 中。
		\pre 断言： <tt>_type(0) < b</tt> 。
		*/
		template<typename _type>
		inline bool
			IsInInterval(_type i, _type b) lnothrow
		{
			LAssert(_type(0) < b,
				"Zero element as lower bound is not less than upper bound.");
			return !(i < _type(0)) && i < b;
		}

		/*!
		\brief 判断 i 是否在左闭右开区间 [a, b) 中。
		\pre 断言： <tt>a < b</tt> 。
		*/
		template<typename _type>
		inline bool
			IsInInterval(_type i, _type a, _type b) lnothrow
		{
			LAssert(a < b, "Lower bound is not less than upper bound.");
			return !(i < a) && i < b;
		}

		/*!
		\brief 判断 i 是否在闭区间 [_type(0), b] 中。
		\pre 断言： <tt>_type(0) < b</tt> 。
		*/
		template<typename _type>
		inline bool
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
		inline bool
			IsInClosedInterval(_type i, _type a, _type b) lnothrow
		{
			LAssert(a < b, "Lower bound is not less than upper bound.");
			return !(i < a || b < i);
		}

		/*!
		\brief 判断 i 是否在开区间 (_type(0), b) 内。
		\pre 断言： <tt>_type(0) < b</tt> 。
		*/
		template<typename _type>
		inline bool
			IsInOpenInterval(_type i, _type b) lnothrow
		{
			LAssert(_type(0) < b,
				"Zero element as lower bound is not less than upper bound.");
			return _type(0) < i && i < b;
		}
		/*!
		\brief 判断 i 是否在开区间 (a, b) 内。
		\pre 断言： <tt>a < b</tt> 。
		*/
		template<typename _type>
		inline bool
			IsInOpenInterval(_type i, _type a, _type b) lnothrow
		{
			LAssert(a < b, "Lower bound is not less than upper bound.");
			return a < i && i < b;
		}

	}
}
