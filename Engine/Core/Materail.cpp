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

			MaterialEvaluator::CheckReductionStatus(ret.second);
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
	for (auto& bind_value : bind_values) {
		bind_effects->GetParameter(bind_value.first) = bind_value.second;
	}
	for (auto & delay_value : delay_values) {
		auto ret = GetInstanceEvaluator().Reduce(delay_value.second);
		if (ret.second == ReductionStatus::Clean)
			bind_effects->GetParameter(delay_value.first) = ret.first.Value.GetContent();
		else
			LF_TraceRaw(Descriptions::Warning, "Material::UpdateParams(pRenderable=%p) ��ֵ%s ��Լʧ��,�������ø�ֵ", pRenderable, bind_effects->GetParameter(delay_value.first).Name.c_str());
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

template<>
std::shared_ptr<Material> platform::AssetResourceScheduler::SyncSpawnResource<Material, const X::path&, const std::string&>(const X::path& path, const std::string & name) {
	auto pAsset = X::LoadMaterialAsset(path);
	if (!pAsset)
		return {};
	auto pMaterial = std::make_shared<Material>(*pAsset, name);
	return pMaterial;
}

template std::shared_ptr<Material> platform::AssetResourceScheduler::SyncSpawnResource<Material, const X::path&, const std::string&>(const X::path& path, const std::string & name);




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

void MaterialEvaluator::CheckReductionStatus(ReductionStatus status)
{
	if (status != ReductionStatus::Clean)
		throw leo::GeneralEvent(leo::sfmt("Bad Reduct State: %s", status == ReductionStatus::Retained ? "Retained" : "Retrying"));
}

void MaterialEvaluator::RegisterMathDotLssFile() {
	lsl::math::RegisterMathDotLssFile(context);
}

