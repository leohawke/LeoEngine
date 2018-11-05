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

namespace lm = leo::math;

static decltype(auto) foo() {

	lm::float2 smq{ 1,0 };
	lm::float3 gugugu{ 1,2,3 };

	gugugu.zx = smq.yx;

	static_assert(sizeof(leo::math::float2) == sizeof(leo::math::float4));

	return 0;
}

