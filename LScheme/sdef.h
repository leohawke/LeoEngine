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

#define YSLIB_COMMIT 9913de30226e25f06a47b5b418330c6567e382e5
#define YSLIB_BUILD 777
#define YSLIB_REV 11

#endif
