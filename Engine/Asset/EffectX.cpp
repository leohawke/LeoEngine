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
		struct EffectDesc {
			X::path effect_path;
			struct Data {
				scheme::TermNode effect_node;
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
			effect_desc.data->effect_node = *LoadNode(effect_desc.effect_path).begin();

			return  nullptr;
		}

		std::shared_ptr<AssetType> ParseNode()
		{
			auto& effect_node = effect_desc.data->effect_node;
			LAssert(leo::Access<std::string>(*effect_node.begin()) == "effect", R"(Invalid Format:Not Begin With "effect")");

			auto SelectNodes = [](const char* name, const scheme::TermNode& node) {
				return node.SelectChildren([&](const scheme::TermNode& child) {
					if (child.size()) {
						return leo::Access<std::string>(*child.begin()) == name;
					}
					return false;
				});
			};

			std::vector<std::pair<std::string, scheme::TermNode>> refers;
			RecursiveReferNode(effect_node,refers);

			auto new_node = leo::MakeNode(MakeIndex(0));

			for (auto& pair : refers) {
				for (auto & node : pair.second) {
					new_node.try_emplace(leo::MakeIndex(new_node),std::make_pair(node.begin(),node.end()), leo::MakeIndex(new_node));
				}
			}

			for(auto & node : effect_node)
				new_node.try_emplace(leo::MakeIndex(new_node), std::make_pair(node.begin(), node.end()), leo::MakeIndex(new_node));

			effect_node = new_node;

			auto Access = [](const char* name, const scheme::TermNode& node) {
				auto it = std::find_if(node.begin(), node.end(), [&](const scheme::TermNode& child) {
					if (child.size())
						return leo::Access<std::string>(*child.begin()) == name;
					return false;
				});
				return leo::Access<std::string>(*(it->rbegin()));
			};

			auto hash = [](auto param) {
				return std::hash<decltype(param)>()(param);
			};

			//macro (macro (name foo) (value bar))
			{
				auto macro_nodes = SelectNodes("macro", effect_node);
				for (auto & macro_node : macro_nodes) {
					effect_desc.effect_asset->GetMacrosRef().emplace_back(
						Access("name", macro_node),
						Access("value", macro_node));
				}
			}
			{
				auto cbuffer_nodes = SelectNodes("cbuffer", effect_node);
				for (auto & cbuffer_node : cbuffer_nodes) {
					asset::EffectConstantBufferAsset cbuffer;
					cbuffer.SetName(Access("name", cbuffer_node));
					auto param_nodes = SelectNodes("parameter", cbuffer_node);
					std::vector<leo::uint32> ParamIndices;
					for (auto & param_node : param_nodes) {
						asset::EffectParameterAsset param;
						param.SetName(Access("name", param_node));
						param.GetTypeRef() = AssetType::GetType(Access("type", param_node));
						if (param.GetType() >= asset::EPT_bool) {
							if (auto p = leo::AccessChildPtr<std::string>(param_node, "arraysize"))
								param.GetArraySizeRef() = std::stoul(*p);
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
				auto fragments = SelectNodes("shader", effect_node);
				for (auto& fragment : fragments) {
					effect_desc.effect_asset->GetFragmentsRef().emplace_back();
					effect_desc.effect_asset->GetFragmentsRef().back().
						GetFragmentRef() = scheme::Deliteralize(
							leo::Access<std::string>(*fragment.rbegin())
						);
				}
			}

			return nullptr;
		}

		std::shared_ptr<AssetType> CreateAsset()
		{
			return effect_desc.effect_asset;
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

		void RecursiveReferNode(const ValueNode& effct_node, std::vector<std::pair<std::string, scheme::TermNode>>& includes) {
			auto refer_nodes = effct_node.SelectChildren([&](const scheme::TermNode& child) {
				if (child.size()) {
					return leo::Access<std::string>(*child.begin()) == "refer";
				}
				return false;
			});
			for (auto & refer_node : refer_nodes) {
				auto path = leo::Access<std::string>(*refer_node.rbegin());

				auto include_node = *LoadNode(path).begin();
				RecursiveReferNode(include_node, includes);

				if (std::find_if(includes.begin(), includes.end(), [&path](const std::pair<std::string,scheme::TermNode>& pair)
				{
					return pair.first == path;
				}
				) == includes.end())
				{
					includes.emplace_back(path, include_node);
				}
			}
		}
	};

	asset::EffectAsset platform::X::LoadEffectAsset(path const & effectpath)
	{
		return *asset::SyncLoad<EffectLoadingDesc>(effectpath);
	}
}
