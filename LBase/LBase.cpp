#include <type_traits>
#include <functional>

using std::invoke_result;

#include "LBase/pseudo_mutex.h"
#include "LBase/scope_gurad.hpp"
#include "LBase/cache.hpp"
#include "LBase/concurrency.h"
#include "LBase/lmathtype.hpp"
#include "LBase/examiner.hpp"
#include "LBase/cast.hpp"
#include "LBase/set.hpp"

static decltype(auto) foo() {
	return 0;
}

