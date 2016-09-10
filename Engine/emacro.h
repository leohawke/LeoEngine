/*! \file Engine\emacro.h
\ingroup Engine
\brief Engine Common Macro
*/
#ifndef LE_emacro_H
#define LE_emacro_H 1


#include <LBase/ldef.h>


/*!
\def LE_DLL
\brief 使用 Render 动态链接库。
\since build 1.02
*/
/*!
\def LE_BUILD_DLL
\brief 构建 Render 动态链接库。
\since build 1.02
*/
/*!
\def LE_API
\brief LBase 应用程序编程接口：用于向库文件约定链接。
\since build 1.02
\todo 判断语言特性支持。
*/
#if defined(LE_DLL) && defined(LE_BUILD_DLL)
#	error "DLL could not be built and used at the same time."
#endif

#if LB_IMPL_MSCPP \
	|| (LB_IMPL_GNUCPP && (defined(__MINGW32__) || defined(__CYGWIN__)))
#	ifdef LB_DLL
#		define LE_API __declspec(dllimport)
#	elif defined(LB_BUILD_DLL)
#		define LE_API __declspec(dllexport)
#	else
#		define LE_API
#	endif
#elif defined(LB_BUILD_DLL) && (LB_IMPL_GNUCPP >= 40000 || LB_IMPL_CLANGPP)
#	define LE_API LB_ATTR(__visibility__("default"))
#else
#	define LE_API
#endif

#endif
