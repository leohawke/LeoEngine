#ifndef IndePlatform_LAssert_h
#define IndePlatform_LAssert_h

#include "ldef.h"
#include <assert.h>

#undef LAssert
#define LAssert(_expr,_msg) assert(_expr)

#ifndef LAssertNonnull
#define LAssertNonnull(_expr) LAssert(bool(_expr), "Null reference found.")
#endif

namespace leo {
	template<typename _type>
	inline _type&&
		Nonnull(_type&& p) lnothrow
	{
		LAssertNonnull(p);
		return lforward(p);
	}
}


#endif
