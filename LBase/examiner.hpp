/*! \file examiner.hpp
\ingroup LBase
\brief C++类型操作检测。
*/

#ifndef LBase_examiner_hpp
#define LBase_examiner_hpp 1

#include "LBase/type_op.hpp"

namespace leo {
	namespace examiners
	{

		/*!
		\brief 基本等于操作检测。
		*/
		struct equal
		{
			template<typename _type1, typename _type2>
			static lconstfn LB_PURE auto
				are_equal(_type1&& x, _type2&& y)
				lnoexcept_spec(bool(x == y)) -> decltype(bool(x == y))
			{
				return bool(x == y);
			}
		};


		/*!
		\brief 基本等于操作检测：总是相等。
		*/
		struct always_equal
		{
			template<typename _type, typename _type2, limpl(typename
				= enable_if_t<!has_equality_operator<_type&&, _type2&&>::value>)>
				static lconstfn LB_STATELESS bool
				are_equal(_type&&, _type2&&) lnothrow
			{
				return true;
			}
		};


		/*!
		\brief 等于操作检测。
		*/
		struct equal_examiner : public equal, public always_equal
		{
			using equal::are_equal;
			using always_equal::are_equal;
		};

	} // namespace examiners;
}


#endif