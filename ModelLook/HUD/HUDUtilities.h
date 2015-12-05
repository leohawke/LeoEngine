#include <ldef.h>
#include <LAssert.h>

namespace  leo
{
	/*!
	\brief 使用 new 复制指定指针指向的对象。
	*/
	template<typename _type>
	lconstfn auto
		CloneNonpolymorphic(const _type& p) -> decltype(&*p)
	{
		return new typename std::remove_reference_t<decltype(*p)>(*p);
	}

	/*!
	\brief 使用 clone 成员函数复制指定指针指向的多态类类型对象。
	\pre 断言： std::is_polymorphic<decltype(*p)>::value 。
	*/
	template<class _type>
	auto
		ClonePolymorphic(const _type& p) -> decltype(&*p)
	{
		static_assert(std::is_polymorphic<std::remove_reference_t<decltype(*p)>>
			::value, "Non-polymorphic class type found.");

		return p->clone();
	}

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
