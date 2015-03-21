////////////////////////////////////////////////////////////////////////////
//
//  Leo Engine Source File.
//  Copyright (C), FNS Studios, 2014-2014.
// -------------------------------------------------------------------------
//  File name:   IndePlatform/ConstexprMath.hpp
//  Version:     v1.00
//  Created:     09/28/2014 by leo hawke.
//  Compilers:   Visual Studio.NET 2013
//  Description: 
// -------------------------------------------------------------------------
//  History:
//		2014-9-28 12:32: vc查找不到函数或无法正确实现constexpr语义<workaround>
//						 编译器必须支持constexpr的语义
//
////////////////////////////////////////////////////////////////////////////
#ifndef IndePlatform_ConstexprMath_Hpp
#define IndePlatform_ConstexprMath_Hpp
#include "ldef.h"
#include "type_op.hpp"
namespace leo
{
	namespace constexprmath
	{
		template<std::uintmax_t base, size_t exp>
		struct pow;

		template<std::uintmax_t base>
		struct pow<base, 0>
		{
#ifdef LB_IMPL_MSCPP
			pow()
			{}
#endif
			static lconstexpr std::uintmax_t value = 1;

#ifndef LB_IMPL_MSCPP
			template<typename T>
			lconstexpr explicit operator T()  lnoexcept
			{
				static_assert(leo::is_integral<T>::value, "T must is integral type");
				return static_cast<T>(value);
			}
#endif
		};

		template<std::uintmax_t base>
		struct pow<base, 1>
		{
#ifdef LB_IMPL_MSCPP
			pow()
			{}
#endif
			static lconstexpr std::uintmax_t value = base;

#ifndef LB_IMPL_MSCPP
			template<typename T>
			lconstexpr explicit operator T()  lnoexcept
			{
				static_assert(leo::is_integral<T>::value, "T must is integral type");
				return static_cast<T>(value);
			}
#endif
		};

		template<std::uintmax_t base, size_t exp>
		struct pow
		{
#ifdef LB_IMPL_MSCPP
			pow()
			{}
#endif

			static lconstexpr std::uintmax_t value = pow<base, exp / 2>::value *pow<base, exp - exp / 2>::value;

#ifndef LB_IMPL_MSCPP
			template<typename T>
			lconstexpr explicit operator T()  lnoexcept
			{
				static_assert(leo::is_integral<T>::value, "T must is integral type");
				return static_cast<T>(value);
			}
#endif
		};
	}

#ifndef LB_IMPL_MSCPP

	template<std::uintmax_t base, size_t exp,typename T>
	constexpr bool operator<(constexprmath::pow<base, exp>, const T& rhs)
	{
		return static_cast<T>(constexprmath::pow<base, exp>::value) < rhs;
	}

	template<std::uintmax_t base, size_t exp, typename T>
	constexpr bool operator>(constexprmath::pow<base, exp>, const T& rhs)
	{
		return static_cast<T>(constexprmath::pow<base, exp>::value) > rhs;
	}


	template<std::uintmax_t base, size_t exp, typename T>
	constexpr bool operator<=(constexprmath::pow<base, exp>, const T& rhs)
	{
		return static_cast<T>(constexprmath::pow<base, exp>::value) <= rhs;
	}

	template<std::uintmax_t base, size_t exp, typename T>
	constexpr bool operator>=(constexprmath::pow<base, exp>, const T& rhs)
	{
		return static_cast<T>(constexprmath::pow<base, exp>::value) >= rhs;
	}

	template<std::uintmax_t base, size_t exp, typename T>
	constexpr bool operator==(constexprmath::pow<base, exp> lhs, const T& rhs)
	{
		return static_cast<T>(constexprmath::pow<base, exp>::value) == rhs;
	}
#endif
}
#endif