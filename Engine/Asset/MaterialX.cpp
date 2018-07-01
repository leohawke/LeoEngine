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

			auto effect_nodes = X::SelectNodes("effect", material_node);
			LAssert(effect_nodes.size() == 1, R"(Invalid Effect Node(begin with "effect") Number)");

			material_desc.effect_name = leo::Access<std::string>(*(effect_nodes.begin()->rbegin()));

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
			for (auto && env_node : X::SelectNodes("env", material_node)) {
				auto path = leo::Access<std::string>(*env_node.rbegin());
				try {
					std::ifstream fin(path);
					material_desc.material_eval->LoadFrom(fin);
				}
				catch (std::invalid_argument& e) {
					LF_TraceRaw(Descriptions::Err, "载入 (env %s) 出现异常:%s",path.c_str(),e.what());
				}
			}

			//env-global


			//丢弃求值器
			material_desc.material_eval.reset();
			return nullptr;
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
