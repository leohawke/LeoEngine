#include "LBase/any.h"
#include <sstream>

#if __STDCPP_THREADS__ || (defined(__GLIBCXX__) \
	&& defined(_GLIBCXX_HAS_GTHREADS) && defined(_GLIBCXX_USE_C99_STDINT_TR1)) \
	|| (defined(_LIBCPP_VERSION) && !defined(_LIBCPP_HAS_NO_THREADS))
#include "LBase/concurrency.h"

namespace leo {

	std::string
		to_string(const std::thread::id& id)
	{
		std::ostringstream oss;

		oss << id;
		return oss.str();
	}
}

#endif