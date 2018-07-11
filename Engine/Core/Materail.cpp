#include "Materail.h"

using namespace platform;

MaterialEvaluator::MaterialEvaluator()
	:LSLEvaluator(MaterialEvalFunctions)
{
}

void MaterialEvaluator::MaterialEvalFunctions(REPLContext& context) {
	using namespace scheme::v1::Forms;
	auto & root(context.Root);
}
