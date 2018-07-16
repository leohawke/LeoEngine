#include "Materail.h"

using namespace platform;
using namespace scheme;
using namespace v1;


MaterialEvaluator::MaterialEvaluator()
	:LSLEvaluator(MaterialEvalFunctions)
{
}

ReductionStatus LazyOnInstance(TermNode& term, ContextNode&) {
	term.Remove(term.begin());

	ValueObject x(MaterialEvaluator::InstanceDelayedTerm(std::move(term)));

	term.Value = std::move(x);

	return ReductionStatus::Clean;
}

ReductionStatus LazyOnRender(TermNode& term, ContextNode&) {
	term.Remove(term.begin());

	ValueObject x(MaterialEvaluator::RenderDelayedTerm(std::move(term)));

	term.Value = std::move(x);

	return ReductionStatus::Clean;
}


void MaterialEvaluator::MaterialEvalFunctions(REPLContext& context) {
	using namespace scheme::v1::Forms;
	auto & root(context.Root);

	RegisterForm(root, "lazy-oninstance", LazyOnInstance);
	RegisterForm(root, "lazy-onrender", LazyOnInstance);
}
