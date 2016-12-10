#include "EffectX.h"
#include "Loader.hpp"

#include<LBase/typeinfo.h>
#include <LScheme/LScheme.h>
#include <memory>
#include <fstream>
#include <iterator>

namespace platform {
	using namespace scheme;

	class EffectLoadingDesc : public asset::AssetLoading<asset::EffectAsset> {
	private:
		struct EffectDesc{
			X::path effect_path;
			struct Data {
				leo::ValueNode effect_node;
			};
			std::shared_ptr<Data> data;
			std::shared_ptr<AssetType> effect_asset;
		} effect_desc;
	public:
		explicit EffectLoadingDesc(X::path const & effectpath)
		{
			effect_desc.effect_path = effectpath;
		}

		std::size_t Type() const override {
			return leo::type_id<EffectLoadingDesc>().hash_code();
		}

		std::experimental::generator<std::shared_ptr<AssetType>> Coroutine() override {
			co_yield PreCreate();
			co_yield LoadNode();
#ifdef ENGINE_TOOL
			co_yield BuildHLSL();
#endif
			co_yield CreateAsset();
		}
	private:
		std::shared_ptr<AssetType> PreCreate()
		{
			effect_desc.data = std::make_shared<EffectDesc::Data>();
			effect_desc.effect_asset = std::make_shared<AssetType>();
			return nullptr;
		}

		std::shared_ptr<AssetType> LoadNode()
		{
			std::ifstream fin(effect_desc.effect_path);
			using sb_it_t = std::istream_iterator<char>;

			scheme::Session session(sb_it_t(fin), sb_it_t{});

			TryExpr(effect_desc.data->effect_node = v1::LoadNode(SContext::Analyze(std::move(session))))
			CatchExpr(..., leo::rethrow_badstate(fin, std::ios_base::failbit))

			return  nullptr;
		}

		std::shared_ptr<AssetType> CreateAsset()
		{
			return effect_desc.effect_asset;
		}
#ifdef ENGINE_TOOL
		std::shared_ptr<AssetType> BuildHLSL()
		{
			return nullptr;
		}
#endif
	};

	asset::EffectAsset platform::X::LoadEffectAsset(path const & effectpath)
	{
		return *asset::SyncLoad<EffectLoadingDesc>(effectpath);
	}
}
