#include <LScheme/LScheme.h>

#include "MaterialX.h"
#include "LSLAssetX.h"
#include "EffectAsset.h"
#include "../Core/Materail.h"
#include "../Core/AssetResourceScheduler.h"

using namespace platform;
using namespace scheme;

namespace details {
	class MaterailLoadingDesc : public asset::AssetLoading<asset::MaterailAsset> {
	private:
		struct MaterailDesc {
			X::path material_path;
			TermNode material_node;
			std::unique_ptr<MaterialEvaluator> material_eval;
			std::shared_ptr<AssetType> material_asset;
			std::shared_ptr<asset::EffectAsset> effect_asset;

			std::string effect_name;
		} material_desc;
	public:
		explicit MaterailLoadingDesc(X::path const & materialpath) {
			material_desc.material_path = materialpath;
			material_desc.material_eval = std::make_unique<MaterialEvaluator>();
		}

		std::size_t Type() const override {
			return leo::type_id<MaterailLoadingDesc>().hash_code();
		}

		std::size_t Hash() const override {
			return leo::hash_combine_seq(Type(), material_desc.material_path.wstring());
		}

		const asset::path& Path() const override {
			return material_desc.material_path;
		}

		std::experimental::generator<std::shared_ptr<AssetType>> Coroutine() override {
			co_yield PreCreate();
			co_yield LoadNode();
			co_yield LoadEffect();
			co_yield ParseNode();
			co_yield CreateAsset();
		}

	private:
		std::shared_ptr<AssetType> PreCreate()
		{
			material_desc.material_asset = std::make_shared<AssetType>();
			return nullptr;
		}

		std::shared_ptr<AssetType> LoadNode()
		{
			material_desc.material_node = *LoadNode(material_desc.material_path).begin();

			auto& material_node = material_desc.material_node;
			LAssert(leo::Access<std::string>(*material_node.begin()) == "material", R"(Invalid Format:Not Begin With "material")");


			//TODO Split
			auto effect_nodes = X::SelectNodes("effect", material_node);
			LAssert(effect_nodes.size() == 1, R"(Invalid Effect Node(begin with "effect") Number)");

			material_desc.effect_name = leo::Access<std::string>(*(effect_nodes.begin()->rbegin()));

			material_node.Remove(effect_nodes.begin()->GetName());

			material_desc.material_asset->GetEffectNameRef() = material_desc.effect_name;
			return  nullptr;
		}

		template<typename path_type>
		scheme::TermNode LoadNode(const path_type& path) {
			std::ifstream fin(path);
			fin >> std::noskipws;
			using sb_it_t = std::istream_iterator<char>;

			scheme::Session session(sb_it_t(fin), sb_it_t{});

			try {
				return SContext::Analyze(std::move(session));
			}

			CatchExpr(..., leo::rethrow_badstate(fin, std::ios_base::failbit));
		}


		std::shared_ptr<AssetType> LoadEffect() {
			material_desc.effect_asset =  X::LoadEffectAsset(material_desc.effect_name + ".lsl");
			return nullptr;
		}

		std::shared_ptr<AssetType> ParseNode()
		{
			auto& material_node = material_desc.material_node;

			//env语句
			//TODO Split
			for (auto && env_node : X::SelectNodes("env", material_node)) {
				auto path = leo::Access<std::string>(*env_node.rbegin());
				material_desc.material_eval->LoadFile(path);
				material_node.Remove(env_node.GetName());
			}

			//env-global
			for (auto && env_node : X::SelectNodes("env-global", material_node)) {
				auto path = leo::Access<std::string>(*env_node.rbegin());
				Material::GetInstanceEvaluator().LoadFile(path);
				material_node.Remove(env_node.GetName());
			}

			//非rem语句
			for (auto && node : material_node.SelectChildren([](const scheme::TermNode& child) {
				if (!child.empty())
					return leo::Access<std::string>(*child.begin()) != "rem";
				return false;
			})) {
				NodeEval(node);
			}

			//丢弃求值器
			material_desc.material_eval.reset();
			return nullptr;
		}

		void NodeEval(const scheme::TermNode& node) {
			//if(StateEval(node))
				//return;

			//variable name
			auto name = leo::Access<std::string>(*node.begin());

			auto& effect_asset = material_desc.effect_asset;
			auto npos = effect_asset->GetParams().size();
			//先查找对应的变量存不存在
			auto param_index =npos;

			auto qualifier_index = name.find('.');

			if (qualifier_index != std::string::npos) {
				string_view cbuffername{ name.data(),qualifier_index };
				string_view paramname(name.data()+ qualifier_index+1,name.length()-1- qualifier_index);

				for (auto & bufferasset : effect_asset->GetCBuffers()) {
					if (bufferasset.GetName() == cbuffername) {
						for (auto & cp_index : bufferasset.GetParamIndices()) {
							if (effect_asset->GetParams()[cp_index].GetName() == paramname) {
								param_index = cp_index;
								break;
							}
						}
						if (param_index != npos)
							break;
					}
				}
			}
			else {
				param_index = std::find_if(effect_asset->GetParams().begin(), effect_asset->GetParams().end(), [&](const asset::EffectParameterAsset& param) {
					return param.GetName() == name;
				}) - effect_asset->GetParams().begin();
			}

			if (param_index == npos) {
				LF_TraceRaw(Descriptions::Warning, "(effect %s) 不存在对应的变量名 (%s ...)", material_desc.effect_name.c_str(), name.c_str());
			}

			//expression
			auto exp = std::move(*node.rbegin());

			try {
				auto ret = material_desc.material_eval->Reduce(exp);

				MaterialEvaluator::CheckReductionStatus(ret.second);
				material_desc.material_asset->GetBindValuesRef().emplace_back(effect_asset->GetParams()[param_index].GetNameHash(), std::move(ret.first.Value));
			}
			catch (std::exception& e) {
				std::stringstream ss;
				scheme::PrintNode(ss, scheme::v1::LoadNode(exp), scheme::LiteralizeEscapeNodeLiteral, [](std::size_t)->std::string {return {}; });
				auto& nodestr = ss.str();
				X::ReduceLFToTab(nodestr, 178);
				//移除连续的\t
				for (auto itr = nodestr.begin(); itr != nodestr.end();) {
					if (*itr == '\t') {
						auto j = std::next(itr);
						if (j != nodestr.end() && *j == ')') {
							itr = nodestr.erase(itr);
							j = std::next(itr);
						}
						while (j != nodestr.end() && *j == '\t')
							j = nodestr.erase(j);
						itr = j;
					}
					else
						++itr;
				}
				LF_TraceRaw(Descriptions::Warning, "变量名:%s 求值(%s)失败 what:%s", name.c_str(), nodestr.c_str(),e.what());
			}
		}

		std::shared_ptr<AssetType> CreateAsset() {
			return material_desc.material_asset;
		}
	};


}

std::shared_ptr<asset::MaterailAsset> X::LoadMaterialAsset(path const & materialpath)
{
	return  AssetResourceScheduler::Instance().SyncLoad<details::MaterailLoadingDesc>(materialpath);
}

std::shared_ptr<Material> X::LoadMaterial(asset::path const& materialpath, const std::string& name) {
	return  AssetResourceScheduler::Instance().SyncSpawnResource<Material>(materialpath, name);
}

