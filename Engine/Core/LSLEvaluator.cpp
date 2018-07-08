#include "LSLEvaluator.h"
#include "LSLBuilder.h"

namespace platform {
	using namespace scheme;
	using namespace v1;

	LSLEvaluator::LSLEvaluator(std::function<void(REPLContext&)> loader)
	:
#ifdef NDEBUG
		context()
#else
		context(true)
#endif
	{
		auto& root(context.Root);

		root.EvaluateLiteral += lsl::context::FetchNumberLiteral();

		loader(context);
	}

	ImplDeDtor(LSLEvaluator)
}