#pragma once

#include <LScheme/LScheme.h>
#include <LFramework/Core/ValueNode.h>
#include <LBase/type_traits.hpp>
#include "../Render/RayTracingDefinitions.h"

#include <filesystem>
#include <fstream>

#include "ShaderAsset.h"
#include "LSLAssetX.h"

namespace platform::X
{
	using leo::ValueNode;
	using path = std::filesystem::path;
	using namespace platform::Render::ShaderCore;

	template<typename AssetType>
	class ShaderLoadingDesc{
	private:
		struct ShaderDesc {
			path path;
			struct Data {
				scheme::TermNode term_node;
			};
			std::shared_ptr<Data> data;
			std::shared_ptr<AssetType> asset;
			std::string shader_code;
			bool is_ray_tracing;
		} shader_desc;

		static_assert(std::is_convertible_v<std::add_pointer_t<AssetType>, std::add_pointer_t<asset::ShadersAsset>>);
	protected:
		explicit ShaderLoadingDesc(X::path const& effectpath)
		{
			shader_desc.path = effectpath;
		}

		std::size_t Hash() const {
			return std::hash<std::wstring>()(shader_desc.path.wstring());
		}

		const path& Path() const{
			return shader_desc.path;
		}

		std::shared_ptr<AssetType> ReturnValue()
		{
			return shader_desc.asset;
		}

		std::add_pointer_t<AssetType> GetAsset()
		{
			return shader_desc.asset.get();
		}

		scheme::TermNode& GetNode()
		{
			return shader_desc.data->term_node;
		}

		std::string& GetCode()
		{
			return shader_desc.shader_code;
		}

		void PreCreate()
		{
			shader_desc.data = std::make_shared<ShaderDesc::Data>();
			shader_desc.asset = std::make_shared<AssetType>();
			shader_desc.asset->SetName(shader_desc.path.string());
			shader_desc.is_ray_tracing = false;
		}

		void LoadNode()
		{
			shader_desc.data->term_node = *LoadNode(shader_desc.path).begin();
		}

		void ParseNode()
		{
			auto& term_node = shader_desc.data->term_node;
			auto tag = leo::Access<std::string>(*term_node.begin());
			shader_desc.is_ray_tracing = tag == "RayTracing";

			LAssert(tag == "effect" || tag == "RayTracing", R"(Invalid Format")");

			std::vector<std::pair<std::string, scheme::TermNode>> refers;
			RecursiveReferNode(term_node, refers);

			auto new_node = leo::MakeNode(leo::MakeIndex(0));

			for (auto& pair : refers) {
				for (auto& node : pair.second) {
					new_node.try_emplace(leo::MakeIndex(new_node), std::make_pair(node.begin(), node.end()), leo::MakeIndex(new_node));
				}
			}

			for (auto& node : term_node)
				new_node.try_emplace(leo::MakeIndex(new_node), std::make_pair(node.begin(), node.end()), leo::MakeIndex(new_node));

			term_node = new_node;

			auto hash = [](auto param) {
				return std::hash<decltype(param)>()(param);
			};

			ParseMacro(shader_desc.asset->GetMacrosRef(), term_node, false);
			ParseConstatnBuffers(term_node);
			//parser params
			{
				auto param_nodes = term_node.SelectChildren([&](const scheme::TermNode& child) {
					if (child.empty())
						return false;
					try {
						return  AssetType::GetType(leo::Access<std::string>(*child.begin())) != SPT_shader;
					}
					catch (leo::unsupported&) {
						return false;
					}
					});
				for (auto& param_node : param_nodes)
					ParseParam(param_node, false);
			}
			{
				auto fragments = X::SelectNodes("shader", term_node);
				for (auto& fragment : fragments) {
					shader_desc.asset->EmplaceShaderGenInfo(AssetType::FRAGMENT, shader_desc.asset->GetFragmentsRef().size(), std::stoul(fragment.GetName()));
					shader_desc.asset->GetFragmentsRef().emplace_back();
					shader_desc.asset->GetFragmentsRef().back().
						GetFragmentRef() = scheme::Deliteralize(
							leo::Access<std::string>(*fragment.rbegin())
						);
				}
			}
			shader_desc.asset->PrepareShaderGen();
			shader_desc.shader_code = shader_desc.asset->GenHLSLShader();
		}

		template<typename path_type>
		scheme::TermNode LoadNode(const path_type& path) {
			std::ifstream fin(path);

			if (!fin.is_open())
				throw;

			fin >> std::noskipws;
			using sb_it_t = std::istream_iterator<char>;

			scheme::Session session(sb_it_t(fin), sb_it_t{});

			try {
				return scheme::SContext::Analyze(std::move(session));
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
			for (auto& refer_node : refer_nodes) {
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

		template<typename _type, typename _func>
		const _type& ReverseAccess(_func f, const scheme::TermNode& node) {
			auto it = std::find_if(node.rbegin(), node.rend(), f);
			return leo::Access<_type>(*it);
		}

		template<typename _type>
		const _type& AccessLastNoChild(const scheme::TermNode& node) {
			return ReverseAccess<_type>([&](const scheme::TermNode& child) {return child.empty(); }, node);
		}

		//根据name判断 取尾部
		std::string Access(const char* name, const scheme::TermNode& node) {
			auto it = std::find_if(node.begin(), node.end(), [&](const scheme::TermNode& child) {
				if (!child.empty())
					return leo::Access<std::string>(*child.begin()) == name;
				return false;
				});
			return leo::Access<std::string>(*(it->rbegin()));
		}

		leo::observer_ptr<const string> AccessPtr(const char* name, const scheme::TermNode& node) {
			auto it = std::find_if(node.begin(), node.end(), [&](const scheme::TermNode& child) {
				if (!child.empty())
					return leo::Access<std::string>(*child.begin()) == name;
				return false;
				});
			if (it != node.end())
				return leo::AccessPtr<std::string>(*(it->rbegin()));
			else
				return nullptr;
		};

		void ParseConstatnBuffers(const scheme::TermNode& term_node)
		{
			auto cbuffer_nodes = SelectNodes("cbuffer", term_node);
			for (auto& cbuffer_node : cbuffer_nodes) {
				asset::ShaderConstantBufferAsset cbuffer;
				cbuffer.SetName(AccessLastNoChild<std::string>(cbuffer_node));

				auto index_space = ParseBind(cbuffer_node);
				cbuffer.SetIndex(index_space.first);
				cbuffer.SetSpace(index_space.second);

				auto param_nodes = cbuffer_node.SelectChildren([&](const scheme::TermNode& child) {
					if (child.empty())
						return false;
					try {
						return  AssetType::GetType(leo::Access<std::string>(*child.begin())) != SPT_shader;
					}
					catch (leo::unsupported&) {
						return false;
					}
					});
				std::vector<leo::uint32> ParamIndices;
				for (auto& param_node : param_nodes) {
					auto param_index = ParseParam(param_node, true);
					ParamIndices.emplace_back(static_cast<leo::uint32>(param_index));
				}
				cbuffer.GetParamIndicesRef() = std::move(ParamIndices);

				shader_desc.asset->EmplaceShaderGenInfo(AssetType::CBUFFER, shader_desc.asset->GetCBuffersRef().size(), std::stoul(cbuffer_node.GetName()));
				shader_desc.asset->GetCBuffersRef().emplace_back(std::move(cbuffer));
			}
			auto cbuffer_template_nodes = SelectNodes("ConstantBuffer", term_node);
			for (auto& cbuffer_node : cbuffer_template_nodes) {
				asset::ShaderConstantBufferAsset cbuffer;
				cbuffer.SetName(AccessLastNoChild<std::string>(cbuffer_node));

				auto index_space = ParseBind(cbuffer_node);
				cbuffer.SetIndex(index_space.first);
				cbuffer.SetSpace(index_space.second);

				if (auto elemtype = AccessPtr("elemtype", cbuffer_node)) {
					cbuffer.GetElemInfoRef() = *elemtype;
				}

				shader_desc.asset->EmplaceShaderGenInfo(AssetType::CBUFFER, shader_desc.asset->GetCBuffersRef().size(), std::stoul(cbuffer_node.GetName()));
				shader_desc.asset->GetCBuffersRef().emplace_back(std::move(cbuffer));
			}
		}

		void ParseMacro(std::vector<asset::ShaderMacro>& macros, const scheme::TermNode& node, bool topmacro)
		{
			//macro (macro (name foo) (value bar))
			auto macro_nodes = X::SelectNodes("macro", node);
			for (auto& macro_node : macro_nodes) {
				if (!topmacro) {
					shader_desc.asset->EmplaceShaderGenInfo(AssetType::MACRO, macros.size(), std::stoul(macro_node.GetName()));
				}
				else {
					//当有其他类型使用index 0时，宏会排在前面
					shader_desc.asset->EmplaceShaderGenInfo(AssetType::MACRO, macros.size(), 0);
				}
				macros.emplace_back(
					Access("name", macro_node),
					Access("value", macro_node));
			}
			UniqueMacro(macros);
		}

		void UniqueMacro(std::vector<asset::ShaderMacro>& macros)
		{
			//TODO! remap shader gen info
			//宏的覆盖原则 删除前面已定义的宏
			auto iter = macros.begin();
			while (iter != macros.end()) {
				auto exist = std::find_if(iter + 1, macros.end(), [&iter](const asset::ShaderMacro& tail)
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

		//Param的名字总是位于Node最后
		size_t ParseParam(const scheme::TermNode& param_node, bool cbuffer_param) {
			asset::ShaderParameterAsset param;
			param.SetName(AccessLastNoChild<std::string>(param_node));
			//don't need check type again
			param.GetTypeRef() = AssetType::GetType(leo::Access<std::string>(*param_node.begin()));

			if (!cbuffer_param) {
				auto index_space = ParseBind(param_node);
				param.SetIndex(index_space.first);
				param.SetSpace(index_space.second);
			}

			if (param.GetType() >= SPT_bool) {
				if (auto p = leo::AccessChildPtr<std::string>(param_node, "arraysize"))
					param.GetArraySizeRef() = std::stoul(*p);
			}
			else if (param.GetType() <= SPT_ConsumeStructuredBuffer) {
				if (auto elemtype = AccessPtr("elemtype", param_node)) {
					try {
						param.GetElemInfoRef() = AssetType::GetType(*elemtype);
					}
					catch (leo::unsupported&) {
						param.GetElemInfoRef() = *elemtype;
					}
				}
			}
			auto index = shader_desc.asset->GetParams().size();
			auto optional_value = ReadParamValue(param_node, param.GetType());
			if (optional_value.has_value())
				shader_desc.asset->BindValue(index, optional_value.value());
			if (!cbuffer_param)
				shader_desc.asset->EmplaceShaderGenInfo(AssetType::PARAM, index, std::stoul(param_node.GetName()));
			shader_desc.asset->GetParamsRef().emplace_back(std::move(param));
			return index;
		}

		std::pair<int,int> ParseBind(const scheme::TermNode& param_node) {
			std::pair<int, int> index_space{asset::BindDesc::Any,asset::BindDesc::Any };

			//(register {index} (space {sapce}))
			if (auto pRegNode = X::SelectNode("register", param_node))
			{
				auto index = leo::Access<std::string>(*std::next(pRegNode->begin()));

				index_space.first = to_uint8(index);

				if (auto pSpaceNode = X::SelectNode("space", param_node))
				{
					auto space = leo::Access<std::string>(*std::next(pSpaceNode->begin()));

					index_space.second = MapSpace(space);
				}
			}
			else if (auto pSpaceNode = X::SelectNode("space", param_node))
			{
				auto space = leo::Access<std::string>(*std::next(pSpaceNode->begin()));

				index_space.second = MapSpace(space);
			}

			return index_space;
		}

		static int MapSpace(const std::string& value)
		{
			auto hash = leo::constfn_hash(value);

			switch (hash)
			{
			case leo::constfn_hash("RAY_TRACING_REGISTER_SPACE_LOCAL"):
				return RAY_TRACING_REGISTER_SPACE_LOCAL;
			case leo::constfn_hash("RAY_TRACING_REGISTER_SPACE_GLOBAL"):
				return RAY_TRACING_REGISTER_SPACE_GLOBAL;
			case leo::constfn_hash("RAY_TRACING_REGISTER_SPACE_SYSTEM"):
				return RAY_TRACING_REGISTER_SPACE_SYSTEM;
			}

			return -1;
		}

		static leo::math::float4 to_float4(const std::string& value) {
			leo::math::float4 result;
			auto iter = result.begin();
			leo::split(value.begin(), value.end(),
				[](char c) {return c == ','; },
				[&](decltype(value.begin()) b, decltype(value.end()) e) {
					auto v = std::string(b, e);
					*iter = std::stof(v);
					++iter;
				}
			);
			return result;
		};

		static leo::uint8 to_uint8(const std::string& value) {
			return static_cast<leo::uint8>(std::stoul(value));
		};

		std::optional<leo::any> ReadParamValue(const scheme::TermNode& param_node, asset::ShaderParamType type) {
#define AccessParam(name,expr) if (auto value = AccessPtr(name, param_node)) expr
			using namespace Render;
			if (type == SPT_sampler) {
				TextureSampleDesc sampler;
				AccessParam("border_clr", sampler.border_clr = M::Color(to_float4(*value).data));

				AccessParam("address_mode_u", sampler.address_mode_u = TextureSampleDesc::to_mode(*value));
				AccessParam("address_mode_v", sampler.address_mode_v = TextureSampleDesc::to_mode(*value));
				AccessParam("address_mode_w", sampler.address_mode_w = TextureSampleDesc::to_mode(*value));

				AccessParam("filtering", sampler.filtering = TextureSampleDesc::to_op<TexFilterOp>(*value));

				AccessParam("max_anisotropy", sampler.max_anisotropy = to_uint8(*value));

				AccessParam("min_lod", sampler.min_lod = std::stof(*value));
				AccessParam("max_lod", sampler.max_lod = std::stof(*value));
				AccessParam("mip_map_lod_bias", sampler.mip_map_lod_bias = std::stof(*value));

				AccessParam("cmp_func", sampler.cmp_func = TextureSampleDesc::to_op<CompareOp>(*value));

				return leo::any(sampler);
			}
#undef AccessParam
			return std::nullopt;
		}
	};
}