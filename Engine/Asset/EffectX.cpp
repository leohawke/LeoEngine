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


			std::vector<std::pair<std::string, scheme::TermNode>> refers;
			RecursiveReferNode(effect_node, refers);

			auto new_node = leo::MakeNode(MakeIndex(0));

			for (auto& pair : refers) {
				for (auto & node : pair.second) {
					new_node.try_emplace(leo::MakeIndex(new_node), std::make_pair(node.begin(), node.end()), leo::MakeIndex(new_node));
				}
			}

			for (auto & node : effect_node)
				new_node.try_emplace(leo::MakeIndex(new_node), std::make_pair(node.begin(), node.end()), leo::MakeIndex(new_node));

			effect_node = new_node;

			auto hash = [](auto param) {
				return std::hash<decltype(param)>()(param);
			};

			ParseMacro(effect_desc.effect_asset->GetMacrosRef(),effect_node);
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
			ParseTechnique(effect_node);

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

				if (std::find_if(includes.begin(), includes.end(), [&path](const std::pair<std::string, scheme::TermNode>& pair)
				{
					return pair.first == path;
				}
				) == includes.end())
				{
					includes.emplace_back(path, include_node);
				}
			}
		}

		typename scheme::TermNode::Container SelectNodes(const char* name, const scheme::TermNode& node) {
			return node.SelectChildren([&](const scheme::TermNode& child) {
				if (child.size()) {
					return leo::Access<std::string>(*child.begin()) == name;
				}
				return false;
			});
		}

		std::string Access(const char* name, const scheme::TermNode& node) {
			auto it = std::find_if(node.begin(), node.end(), [&](const scheme::TermNode& child) {
				if (child.size())
					return leo::Access<std::string>(*child.begin()) == name;
				return false;
			});
			return leo::Access<std::string>(*(it->rbegin()));
		}

		leo::observer_ptr<const string> AccessPtr(const char* name, const scheme::TermNode& node) {
			auto it = std::find_if(node.begin(), node.end(), [&](const scheme::TermNode& child) {
				if (child.size())
					return leo::Access<std::string>(*child.begin()) == name;
				return false;
			});
			if (it != node.end())
				return leo::AccessPtr<std::string>(*(it->rbegin()));
			else
				return nullptr;
		};

		void ParseMacro(std::vector<asset::EffectMacro>& macros, const scheme::TermNode& node)
		{
			//macro (macro (name foo) (value bar))
			auto macro_nodes = SelectNodes("macro", node);
			for (auto & macro_node : macro_nodes) {
				macros.emplace_back(
					Access("name", macro_node),
					Access("value", macro_node));
			}
			UniqueMacro(macros);
		}

		void ParseTechnique(const scheme::TermNode& effect_node)
		{
			auto techniques = SelectNodes("technique", effect_node);
			for (auto & technique_node : techniques) {
				asset::EffectTechniqueAsset technique;
				technique.SetName(Access("name", technique_node));
				auto inherit = AccessPtr("inherit", technique_node);
				if (inherit) {
					auto exist_techniques = effect_desc.effect_asset->GetTechniques();
					auto inherit_technique = std::find_if(exist_techniques.begin(), exist_techniques.end(), [&inherit](const asset::EffectTechniqueAsset& exist)
					{
						return exist.GetName() == *inherit;
					});
					LAssert(inherit_technique != exist_techniques.end(), "Can't Find Inherit Technique");
					technique.GetMacrosRef() = inherit_technique->GetMacros();
					//继承technique意味继承所有的pass
					technique.GetPassesRef() = inherit_technique->GetPasses();
				}
				ParseMacro(technique.GetMacrosRef(), technique_node);

				auto passes = SelectNodes("pass", technique_node);
				for (auto & pass_node : passes) {
					asset::TechniquePassAsset pass;
					pass.SetName(Access("name", pass_node));

					asset::TechniquePassAsset* pass_ptr = nullptr;
					if (inherit) {
						auto exist_passes = technique.GetPasses();
						auto exist_pass = std::find_if(exist_passes.begin(), exist_passes.end(), [&pass](const asset::TechniquePassAsset& exist)
						{
							return pass.GetNameHash() == exist.GetNameHash();
						}
						);
						if (exist_pass != exist_passes.end())
							pass_ptr = &(*exist_pass);
					}
					if (!pass_ptr)
					{
						//找不到已知的pass构造一个新的
						technique.GetPassesRef().emplace_back();
						pass_ptr = &technique.GetPassesRef().back();
						pass_ptr->SetName(pass.GetName());
					}

					//pass本身还能继承
					auto inherit_pass = AccessPtr("inherit", pass_node);
					if (inherit_pass) {
						auto exist_passes = technique.GetPasses();
						auto exist_pass = std::find_if(exist_passes.begin(), exist_passes.end(), [&inherit_pass](const asset::TechniquePassAsset& exist)
						{
							return *inherit_pass == exist.GetName();
						}
						);
						LAssert(exist_pass != exist_passes.end(), "Can't Find Inherit Technique");
						pass_ptr->GetMacrosRef() = exist_pass->GetMacros();
						pass_ptr->GetPipleStateRef() = exist_pass->GetPipleState();
					}

					//pass也有宏节点
					ParseMacro(pass.GetMacrosRef(), pass_node);
					ParsePassState(*pass_ptr, pass_node);
				}
				effect_desc.effect_asset->GetTechniquesRef().emplace_back(std::move(technique));
			}
		}

		void ParsePassState(asset::TechniquePassAsset& pass, scheme::TermNode& pass_node)
		{

		}

		void UniqueMacro(std::vector<asset::EffectMacro>& macros)
		{
			//宏的覆盖原则 删除前面已定义的宏
			auto iter = macros.begin();
			while (iter != macros.end()) {
				auto exist = std::find_if(iter + 1, macros.end(), [&iter](const asset::EffectMacro& tail)
				{
					return iter->first == tail.first;
				}
				);
				if (exist != macros.end())
					iter = macros.erase(iter);
				else
					++iter;
			}
		}
	};

	asset::EffectAsset platform::X::LoadEffectAsset(path const & effectpath)
	{
		return *asset::SyncLoad<EffectLoadingDesc>(effectpath);
	}
}
