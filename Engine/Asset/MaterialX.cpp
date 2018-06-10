#include <LScheme/LScheme.h>

#include "MaterialX.h"
#include "../Core/AssetResourceScheduler.h"

using namespace platform;
using namespace scheme;

namespace details {
	class MaterailLoadingDesc : public asset::AssetLoading<asset::MaterailAsset> {
	private:
		struct MaterailDesc {
			X::path material_path;
			TermNode material_node;
			//std::shared_ptr<Enviroment> material_env;
			std::shared_ptr<AssetType> material_asset;
		} material_desc;
	public:
		explicit MaterailLoadingDesc(X::path const & materialpath) {
			material_desc.material_path = materialpath;
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

			CatchExpr(..., leo::rethrow_badstate(fin, std::ios_base::failbit))
		}

		std::shared_ptr<AssetType> ParseNode()
		{
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
