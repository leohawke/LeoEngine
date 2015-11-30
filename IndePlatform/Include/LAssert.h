#ifndef IndePlatform_LAssert_h
#define IndePlatform_LAssert_h

#include <assert.h>

#undef LAssert
#define LAssert(_expr,_msg) assert(_expr)

#endif
