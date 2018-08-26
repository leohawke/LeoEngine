#include "Materail.h"
#include "LSLBuilder.h"
#include "AssetResourceScheduler.h"
#include "../Asset/MaterialX.h"
using namespace platform;
using namespace scheme;
using namespace v1;

platform::Material::Material(const asset::MaterailAsset & asset, const std::string & name)
	:identity_name(asset::path(AssetResourceScheduler::Instance().FindAssetPath(&asset)).replace_extension().u8string() + "-" + name)
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
		else if (LB_UNLIKELY(bind_value.second.GetContent().type() == leo::type_id<MaterialEvaluator::RenderDelayedTerm>())) {
			delay_values.emplace_back(bind_value.first,
				scheme::TermNode(bind_value.second.Access<MaterialEvaluator::RenderDelayedTerm>()));
		}
		else{
			bind_values.emplace_back(bind_value);
		}
	}
}

void platform::Material::UpdateParams(Renderable* pRenderable) {

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

template<>
std::shared_ptr<Material> AssetResourceScheduler::SyncSpawnResource<Material, const X::path&, const std::string&>(const X::path& path, const std::string & name) {
	auto pAsset = X::LoadMaterialAsset(path);
	if (!pAsset)
		return {};
	auto pMaterial = std::make_shared<Material>(*pAsset, name);
	return pMaterial;
}

template std::shared_ptr<Material> AssetResourceScheduler::SyncSpawnResource<Material, const X::path&, const std::string&>(const X::path& path, const std::string & name);




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

