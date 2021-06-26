#include <type_traits>
#include <functional>

using std::invoke_result;

#include "LBase/pseudo_mutex.h"
#include "LBase/scope_gurad.hpp"
#include "LBase/cache.hpp"
#include "LBase/concurrency.h"
#include "LBase/examiner.hpp"
#include "LBase/cast.hpp"
#include "LBase/set.hpp"
#include "LBase/lmath.hpp"
#include "LBase/ConcurrentHashMap.h"

namespace lm = leo::math;

static decltype(auto) foo() {

	leo::ConcurrentHashMap<int, int> s{};

	return 0;
}

