#pragma once
#include <LBase/cassert.h>

#define FOLLY_SAFE_CHECK(expr, ...)  LAssert(expr,"Assertion failure.")
#define FOLLY_SAFE_DCHECK FOLLY_SAFE_CHECK