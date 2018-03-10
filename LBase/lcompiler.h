#ifndef LBase_lcompiler_h
#define LBase_lcompiler_h 1
#include "LBase/ldef.h"

/*!
\brief \<functional\> 特性测试宏。
\see WG21 P0096R1 3.5 。
\since build 1.4
*/
//@{
#if LB_IMPL_MSCPP >= 1800  || defined(__clang__)
#	ifndef __cpp_lib_transparent_operators
#		define __cpp_lib_transparent_operators 201210
#	endif
#endif
//@}


#if LB_IMPL_MSCPP >= 1900 || defined(__clang__)
//! \see https://blogs.msdn.microsoft.com/vcblog/2015/06/19/c111417-features-in-vs-2015-rtm/ 。
//@{
/*!
\brief \<type_traits\> 特性测试宏。
\see WG21 P0096R1 3.4 。
\since build 1.4
*/
//@{
#	ifndef __cpp_lib_bool_constant
#		define __cpp_lib_bool_constant 201505
#	endif
#	ifndef __cpp_lib_void_t
#		define __cpp_lib_void_t 201411
#	endif
//@}
/*!
\brief \<utility\> 特性测试宏。
\see WG21 P0096R1 3.5 。
\since build 1.4
*/
//@{
#	ifndef __cpp_lib_integer_sequence
#		define __cpp_lib_integer_sequence 201304
#	endif
#	ifndef __cpp_lib_exchange_function
#		define __cpp_lib_exchange_function 201304
#	endif
#	ifndef __cpp_lib_tuple_element_t
#		define __cpp_lib_tuple_element_t 201402
#	endif
#	ifndef	__cpp_lib_transformation_trait_aliases
		#define __cpp_lib_transformation_trait_aliases 201304
	#endif

#	ifndef __cpp_lib_make_reverse_iterator
#		define __cpp_lib_make_reverse_iterator 201402
#	endif

#	ifndef __cpp_lib_optional
#		define __cpp_lib_optional 201411
#	endif
//@}

/*!
\brief \<functional\> 特性测试宏。
\see WG21 P0096R1 3.5 。
\since build 1.4
*/
//@{
#	ifndef __cpp_lib_invoke
#		define __cpp_lib_invoke 201411
#	endif
//@}
//@}
#endif

#ifdef LB_IMPL_MSCPP
#define oper_bool(value) lconstfn operator bool() const lnoexcept {return value;}
#else
#define oper_bool(...)
#endif

#endif