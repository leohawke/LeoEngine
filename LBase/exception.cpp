#include "LBase/exception.h"

namespace leo {
	unsupported::~unsupported() = default;


	unimplemented::~unimplemented() = default;

	invalid_construction::invalid_construction()
		: invalid_argument("Violation on construction found.")
	{}

	invalid_construction::~invalid_construction() = default;

	void
		throw_invalid_construction()
	{
		throw invalid_construction();
	}
}