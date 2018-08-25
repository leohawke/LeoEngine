#include "Materail.h"
#include "LSLBuilder.h"
using namespace platform;
using namespace scheme;
using namespace v1;

platform::Material::Material(const asset::MaterailAsset & asset, const std::string & name)
{
	for (auto& bind_value : asset.GetBindValues()) {
		if (LB_UNLIKELY(bind_value.second.GetContent().type() == leo::type_id<MaterialEvaluator::InstanceDelayedTerm>())){
			auto ret = GetInstanceEvaluator().Reduce(bind_value.second.Access<MaterialEvaluator::InstanceDelayedTerm>());

			if (ret.second != ReductionStatus::Clean)
				throw leo::GeneralEvent(leo::sfmt("Bad Reduct State: %s", ret.second == ReductionStatus::Retained ? "Retained" : "Retrying"));

			bind_values.emplace_back(bind_value.first,
				ret.first.Value.GetContent()
			);
		}
		else {
			bind_values.emplace_back(bind_value);
		}
	}
}

MaterialEvaluator & platform::Material::GetInstanceEvaluator()
{
	static MaterialEvaluator instance{};

	struct InitBlock final
	{
		InitBlock(MaterialEvaluator& evaluator) {
			auto& context = evaluator.context;
			auto & root(context.Root);

			//TODO add global variable
		}
	};

	static leo::call_once_init<InitBlock, leo::once_flag> init{ instance };

	return instance;
}


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

void MaterialEvaluator::RegisterMathDotLssFile() {
	lsl::math::RegisterMathDotLssFile(context);
}

