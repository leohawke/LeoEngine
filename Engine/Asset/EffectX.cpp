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
			co_yield ParseNode();
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

		std::shared_ptr<AssetType> ParseNode()
		{
			auto& effect_node = effect_desc.data->effect_node;
			LAssert(effect_node.GetName() == "effect", R"(Invalid Format:Not Begin With "effect")");

			//TODO refer support

			auto SelectNodes = [](const char* name,const ValueNode& node) {
				return node.SelectChildren([&](const leo::ValueNode& child) {
					return child == name;
				});
			};

			auto Access = [](const char* name, const ValueNode& node) {
				return leo::AccessChild<std::string>(node,name);
			};

			auto hash = [](auto param) {
				return std::hash<decltype(param)>()(param);
			};

			//macro (macro (name foo) (value bar))
			{
				auto macro_nodes = SelectNodes("macro",effect_node);
				for (auto & macro_node : macro_nodes) {
					effect_desc.effect_asset->GetMacrosRef().emplace_back(
						Access("name",macro_node),
						Access("value", macro_node));
				}
			}
			{
				auto cbuffer_nodes = SelectNodes("cbuffer",effect_node);
				for (auto & cbuffer_node : cbuffer_nodes) {
					asset::EffectConstantBufferAsset cbuffer;
					cbuffer.SetName(Access("name", cbuffer_node));
					auto param_nodes = SelectNodes("parameter", cbuffer_node);
					std::vector<leo::uint32> ParamIndices;
					for (auto & param_node : param_nodes) {
						asset::EffectParameterAsset param;
						param.SetName(Access("name", param_node));
						param.GetTypeRef() = AssetType::GetType(Access("type",param_node));
						if (param.GetType() >= asset::EPT_bool) {
							param.GetArraySizeRef() =std::stoul(Access("arraysize", param_node));
						}
						else if (param.GetType() <= asset::EPT_consume_structured_buffer) {
							param.GetElemTypeRef() = AssetType::GetType(Access("elemtype", param_node));
						}
						ParamIndices.emplace_back(static_cast<leo::uint32>(effect_desc.effect_asset->GetParams().size()));
						effect_desc.effect_asset->GetParamsRef().emplace_back(std::move(param));
					}
					cbuffer.GetParamIndicesRef() = std::move(ParamIndices);
					effect_desc.effect_asset->GetCBuffersRef().emplace_back(std::move(cbuffer));
				}
			}
			{
				auto fragment= leo::AccessChild<std::string>(effect_node, "shader");
				effect_desc.effect_asset->GetFragmentsRef().emplace_back();
				effect_desc.effect_asset->GetFragmentsRef().back().GetFragmentRef() = fragment;
			}

			return nullptr;
		}

		std::shared_ptr<AssetType> CreateAsset()
		{
			return effect_desc.effect_asset;
		}
	};

	asset::EffectAsset platform::X::LoadEffectAsset(path const & effectpath)
	{
		return *asset::SyncLoad<EffectLoadingDesc>(effectpath);
	}
}
