#include "LSLEvaluator.h"

namespace platform {
	LSLEvaluator::LSLEvaluator(std::function<void(REPLContext&)> loader)
	:
#ifdef NDEBUG
		context()
#else
		context(true)
#endif
	{
		loader(context);
	}

	ImplDeDtor(LSLEvaluator)
}