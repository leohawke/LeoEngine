/*! \file cassert.h
\ingroup LBase
\breif ISO C 断言/调试跟踪扩展。
\note 默认输出使用操作系统调试信息输出接口
\todo 基于接口扩展控制台输出，文件输出
*/

#ifndef LBase_cassert_h
#define LBase_cassert_h 1

#include "LBase/ldef.h"
#include <cstdio>

namespace platform {
	/*!
	\brief LBase 默认Debug输出函数
	\note 该函数使用操作系统调试信息输出接口
	\note 对于没有的接口，将会定位至wcerr/cerr/stderr
	\note 该函数用于输出"鸡肋"级别的信息[会输出所有信息至此]
	*/
	LB_API void
		ldebug(const char*, ...) lnothrow;

	LB_API void
		ldebug(const wchar_t*, ...) lnothrow;
}


namespace leo
{
	/*!
	\brief LBase 默认断言函数。
	\note 当定义宏 LB_Use_LAssert 不等于 0 时，宏 LAssert 操作由此函数实现。
	\note 参数依次为：表达式、文件名、行号和消息文本。
	\note 允许空指针参数，视为未知。
	\note 调用 std::terminate 终止程序。
	*/
	LB_NORETURN LB_API void
		lassert(const char*, const char*, int, const char*) lnothrow;

#if LB_Use_LTrace
	/*!
	\brief LBase 调试跟踪函数。
	\note 当定义宏 LB_Use_YTrace 不等于 0 时，宏 LTrace 操作由此函数实现。
	*/
	LB_API  void
		ltrace(std::FILE*, std::uint8_t, std::uint8_t, const char*, int, const char*,
			...) lnothrow;
#endif

} // namespace leo;

#include <cassert>

#undef lconstraint
#undef lassume

  /*!
  \ingroup LBase_pseudo_keyword
  \def lconstraint
  \brief 约束：接口语义。
  \note 和普通断言相比强调接口契约。对移植特定的平台实现时应予以特别注意。
  \note 保证兼容 ISO C++11 constexpr 模板。
  \see $2015-10 @ %Documentation::Workflow::Annual2015.

  运行时检查的接口语义约束断言。不满足此断言的行为是接口明确地未定义的，行为不可预测。
  */
#ifdef NDEBUG
#define ldebug(format,...) (void)0
#else
#define ldebug(format,...) ::platform::ldebug(format,__VA_ARGS__)
#endif

#ifdef NDEBUG
#	define lconstraint(_expr) LB_ASSUME(_expr)
#else
#	define lconstraint(_expr) \
	((_expr) ? void(0) : leo::lassert(#_expr, __FILE__, __LINE__, \
		"Constraint violation."))
#endif

  /*!
  \ingroup LBase_pseudo_keyword
  \def lassume
  \brief 假定：环境语义。
  \note 和普通断言相比强调非接口契约。对移植特定的平台实现时应予以特别注意。

  运行时检查的环境条件约束断言。用于明确地非 lconstraint 适用的情形。
  */
#ifdef NDEBUG
#	define lassume(_expr) LB_ASSUME(_expr)
#else
#	define lassume(_expr) assert(_expr)
#endif


#ifndef LAssert
#	if LB_Use_LAssert
#		define LAssert(_expr, _msg) \
	((_expr) ? void(0) : leo::lassert(#_expr, __FILE__, __LINE__, _msg))
#	else
#		define LAssert(_expr, _msg) lassume(_expr)
#	endif
#endif
#pragma warning(disable:4800)
#ifndef LAssertNonnull
#	define LAssertNonnull(_expr) LAssert(bool(_expr), "Null reference found.")
#endif

#ifndef LTrace
  //@{
  /*!
  \brief LBase 扩展调试跟踪。
  \note 使用自定义的调试跟踪级别。
  \sa ltrace
  */
#	if LB_Use_LTrace
#		define LTrace(_stream, _lv, _t, _msg, ...) \
	leo::ltrace(_stream, _lv, _t, __FILE__, __LINE__, _msg, __VA_ARGS__)
#	else
#		define LTrace(...)
#	endif
#endif
#endif
