#ifndef LScheme_SDEF_H
#define LScheme_SDEF_H 1

#include <LBase/ldef.h>

#if defined(LS_DLL) && defined(LS_BUILD_DLL)
#	error "DLL could not be built and used at the same time."
#endif

#if LB_IMPL_MSCPP \
	|| (LB_IMPL_GNUCPP && (defined(__MINGW32__) || defined(__CYGWIN__)))
#	ifdef LS_DLL
#		define LS_API __declspec(dllimport)
#	elif defined(LS_BUILD_DLL)
#		define LS_API __declspec(dllexport)
#	else
#		define LS_API
#	endif
#elif defined(LS_BUILD_DLL) && (LB_IMPL_GNUCPP >= 40000 || LB_IMPL_CLANGPP)
#	define LS_API LB_ATTR(__visibility__("default"))
#else
#	define LS_API
#endif

#define YSLIB_COMMIT dc0317b085bc99e8965b570ced94c00bfda9cd01
#define YSLIB_BUILD 739
#define YSLIB_REV 11

#endif
