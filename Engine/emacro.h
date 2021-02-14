/*! \file Engine\emacro.h
\ingroup Engine
\brief Engine Common Macro
*/
#ifndef LE_emacro_H
#define LE_emacro_H 1


#include <LBase/cformat.h>
#include <LFramework/LCLib/Debug.h>

/*!
\def LE_DLL
\brief ʹ�� Engine ��̬���ӿ⡣
\since build 1.02
*/
/*!
\def LE_BUILD_DLL
\brief ���� Engine ��̬���ӿ⡣
\since build 1.02
*/
/*!
\def LE_API
\brief LBase Ӧ�ó����̽ӿڣ���������ļ�Լ�����ӡ�
\since build 1.02
\todo �ж���������֧�֡�
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

#ifndef LE_LogError
#define LE_LogError(...) platform_ex::SendDebugString(platform::Descriptions::Err,leo::sfmt(__VA_ARGS__).c_str())
#endif

#ifndef LE_LogWarning
#define LE_LogWarning(...) platform_ex::SendDebugString(platform::Descriptions::Warning,leo::sfmt(__VA_ARGS__).c_str())
#endif

#endif
