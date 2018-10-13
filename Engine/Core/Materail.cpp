#include "Materail.h"
#include "LSLBuilder.h"
#include "AssetResourceScheduler.h"
#include "../Asset/MaterialX.h"
using namespace platform;
using namespace scheme;
using namespace v1;

namespace fs = std::experimental::filesystem;

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
			LF_TraceRaw(Descriptions::Warning, "Material::UpdateParams(pRenderable=%p) 求值%s 规约失败,放弃设置该值", pRenderable, bind_effects->GetParameter(delay_value.first).Name.c_str());
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
	RegisterForm(root, "lazy-onrender", LazyOnRender);
}

void MaterialEvaluator::CheckReductionStatus(ReductionStatus status)
{
	if (status != ReductionStatus::Clean)
		throw leo::GeneralEvent(leo::sfmt("Bad Reduct State: %s", status == ReductionStatus::Retained ? "Retained" : "Retrying"));
}

void MaterialEvaluator::RegisterMathDotLssFile() {
	lsl::math::RegisterMathDotLssFile(context);
}

void MaterialEvaluator::LoadFile(const path & filepath)
{
	auto hash_value = fs::hash_value(filepath);
	if (loaded_fileshash_set.find(hash_value) != loaded_fileshash_set.end())
		return;
	try {
		if (filepath == "math.lss") {
			RegisterMathDotLssFile();
		}
		else {
			std::ifstream fin(filepath);
			LoadFrom(fin);
		}
		loaded_fileshash_set.emplace(hash_value);
	}
	catch (std::invalid_argument& e) {
		LF_TraceRaw(Descriptions::Err, "载入 (env %s) 出现异常:%s", filepath.c_str(), e.what());
	}
}

void MaterialEvaluator::Define(string_view id,ValueObject && vo, bool forced)
{
	auto & root(context.Root);
	auto& root_env(root.GetRecordRef());
	root_env.Define(id,std::move(vo), forced);
}

