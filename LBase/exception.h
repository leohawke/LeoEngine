/*! \file exception.h
\ingroup LBase
\brief 标准库异常扩展接口。
*/

#ifndef LBase_exception_h
#define LBase_exception_h 1

#include "LBase/deref_op.hpp" // for call_value_or;
#include "LBase/type_traits.hpp" // for remove_cv_t;
#include <memory> // for std::addressof;
#include <functional> // for std::mem_fn;
#include <stdexcept> // for std::logic_error;
#include <system_error> // for std::error_category, std::generic_category,
//	std::system_error, std::errc;
#include "exception_type.h"

namespace leo
{
	/*!	\defgroup exceptions Exceptions
	\brief 异常类型。
	\since build 1.4
	*/


	/*!
	\brief 迭代处理异常。
	\since build 1.4
	*/
	template<typename _func>
	void
		iterate_exceptions(_func&& f, std::exception_ptr p = std::current_exception())
	{
		while (p)
			p = lforward(f)(p);
	}

	/*!
	\brief 取嵌套异常指针。
	\return 若符合 std::rethrow_if_nested 抛出异常的条件则返回异常指针，否则为空。
	\since build 1.4
	*/
	template<class _tEx>
	inline std::exception_ptr
		get_nested_exception_ptr(const _tEx& e)
	{
		return leo::call_value_or(std::mem_fn(&std::nested_exception::nested_ptr),
			dynamic_cast<const std::nested_exception*>(std::addressof(e)));
	}

	/*!
	\brief 引发异常：若当前存在正在处理的异常则抛出嵌套异常，否则抛出异常。
	\since build 1.4
	*/
	template<typename _type>
	LB_NORETURN inline void
		raise_exception(_type&& e)
	{
		if (std::current_exception())
			std::throw_with_nested(lforward(e));
		throw lforward(e);
	}


	/*!
	\brief 处理嵌套异常。
	\since build 1.4
	*/
	template<typename _tEx, typename _func,
		typename _tExCaught = remove_cv_t<_tEx>&>
		void
		handle_nested(_tEx& e, _func&& f)
	{
		try
		{
			std::rethrow_if_nested(e);
		}
		catch (_tExCaught ex)
		{
			lforward(f)(ex);
		}
	}

	
	/*!
	\brief 抛出 invalid_construction 异常。
	\throw invalid_construction
	\relates invalid_construction
	*/
	LB_NORETURN LB_API void
		throw_invalid_construction();

	LB_NORETURN LB_API void
		throw_out_of_range(const char* msg);

	/*!
	\brief 抛出错误： std::system_error 或允许相同构造函数参数的异常。
	\throw _type 使用参数构造的异常。
	\since build 1.4
	*/
	//@{
	template<class _type = std::system_error, typename... _tParams>
	LB_NORETURN inline void
		throw_error(std::error_code ec, _tParams&&... args)
	{
		throw _type(ec, lforward(args)...);
	}
	template<class _type = std::system_error, typename... _tParams>
	LB_NORETURN inline void
		throw_error(std::error_condition cond, _tParams&&... args)
	{
		throw _type(cond.value(), cond.category(), lforward(args)...);
	}
	template<class _type = std::system_error, typename... _tParams>
	LB_NORETURN inline void
		throw_error(int ev, _tParams&&... args)
	{
		throw _type(ev, std::generic_category(), lforward(args)...);
	}
	template<class _type = std::system_error, typename... _tParams>
	LB_NORETURN inline void
		throw_error(int ev, const std::error_category& ecat, _tParams&&... args)
	{
		throw _type(ev, ecat, lforward(args)...);
	}
	//@}

} // namespace leo;
#endif