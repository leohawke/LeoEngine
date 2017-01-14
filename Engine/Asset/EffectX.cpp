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

			ParseMacro(effect_desc.effect_asset->GetMacrosRef(), effect_node);
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
			using leo::constfn_hash;
			using namespace Render;
			auto to_bool = [](const std::string& value) {
				return value.size() > 0 && value[0] == 't';
			};

			auto to_uint8 = [](const std::string& value) {
				return static_cast<leo::uint8>(std::stoul(value));
			};

			auto to_uint16 = [](const std::string& value) {
				return static_cast<leo::uint16>(std::stoul(value));
			};

			auto to_uint32 = [](const std::string& value) {
				return static_cast<leo::uint32>(std::stoul(value));
			};

			auto to_float4 = [](const std::string& value) {
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

			auto & rs_desc = pass.GetPipleStateRef().RasterizerState;
			auto & ds_desc = pass.GetPipleStateRef().DepthStencilState;
			auto & bs_desc = pass.GetPipleStateRef().BlendState;

			auto default_parse = [&](std::size_t hash, const std::string& value) {
				switch (hash)
				{
				case constfn_hash("blend_enable_0"):
					bs_desc.blend_enable[0] = to_bool(value);
					break;
				case constfn_hash("blend_enable_1"):
					bs_desc.blend_enable[1] = to_bool(value);
					break;
				case constfn_hash("blend_enable_2"):
					bs_desc.blend_enable[2] = to_bool(value);
					break;
				case constfn_hash("blend_enable_3"):
					bs_desc.blend_enable[3] = to_bool(value);
					break;
				case constfn_hash("blend_enable_4"):
					bs_desc.blend_enable[4] = to_bool(value);
					break;
				case constfn_hash("blend_enable_5"):
					bs_desc.blend_enable[5] = to_bool(value);
					break;
				case constfn_hash("blend_enable_6"):
					bs_desc.blend_enable[6] = to_bool(value);
					break;
				case constfn_hash("blend_enable_7"):
					bs_desc.blend_enable[7] = to_bool(value);
					break;
				case constfn_hash("logic_op_enable_0"):
					bs_desc.logic_op_enable[0] = to_bool(value);
					break;
				case constfn_hash("logic_op_enable_1"):
					bs_desc.logic_op_enable[1] = to_bool(value);
					break;
				case constfn_hash("logic_op_enable_2"):
					bs_desc.logic_op_enable[2] = to_bool(value);
					break;
				case constfn_hash("logic_op_enable_3"):
					bs_desc.logic_op_enable[3] = to_bool(value);
					break;
				case constfn_hash("logic_op_enable_4"):
					bs_desc.logic_op_enable[4] = to_bool(value);
					break;
				case constfn_hash("logic_op_enable_5"):
					bs_desc.logic_op_enable[5] = to_bool(value);
					break;
				case constfn_hash("logic_op_enable_6"):
					bs_desc.logic_op_enable[6] = to_bool(value);
					break;
				case constfn_hash("logic_op_enable_7"):
					bs_desc.logic_op_enable[7] = to_bool(value);
					break;

				case constfn_hash("blend_op_0"):
					bs_desc.blend_op[0] = BlendDesc::to_op(value);
					break;
				case constfn_hash("blend_op_1"):
					bs_desc.blend_op[1] = BlendDesc::to_op(value);
					break;
				case constfn_hash("blend_op_2"):
					bs_desc.blend_op[2] = BlendDesc::to_op(value);
					break;
				case constfn_hash("blend_op_3"):
					bs_desc.blend_op[3] = BlendDesc::to_op(value);
					break;
				case constfn_hash("blend_op_4"):
					bs_desc.blend_op[4] = BlendDesc::to_op(value);
					break;
				case constfn_hash("blend_op_5"):
					bs_desc.blend_op[5] = BlendDesc::to_op(value);
					break;
				case constfn_hash("blend_op_6"):
					bs_desc.blend_op[6] = BlendDesc::to_op(value);
				case constfn_hash("blend_op_7"):
					bs_desc.blend_op[7] = BlendDesc::to_op(value);
					break;

				case constfn_hash("src_blend_0"):
					bs_desc.src_blend[0] = BlendDesc::to_factor(value);
					break;
				case constfn_hash("src_blend_1"):
					bs_desc.src_blend[1] = BlendDesc::to_factor(value);
					break;
				case constfn_hash("src_blend_2"):
					bs_desc.src_blend[2] = BlendDesc::to_factor(value);
					break;
				case constfn_hash("src_blend_3"):
					bs_desc.src_blend[3] = BlendDesc::to_factor(value);
					break;
				case constfn_hash("src_blend_4"):
					bs_desc.src_blend[4] = BlendDesc::to_factor(value);
					break;
				case constfn_hash("src_blend_5"):
					bs_desc.src_blend[5] = BlendDesc::to_factor(value);
					break;
				case constfn_hash("src_blend_6"):
					bs_desc.src_blend[6] = BlendDesc::to_factor(value);
				case constfn_hash("src_blend_7"):
					bs_desc.src_blend[7] = BlendDesc::to_factor(value);
					break;
				case constfn_hash("dst_blend_0"):
					bs_desc.dst_blend[0] = BlendDesc::to_factor(value);
					break;
				case constfn_hash("dst_blend_1"):
					bs_desc.dst_blend[1] = BlendDesc::to_factor(value);
					break;
				case constfn_hash("dst_blend_2"):
					bs_desc.dst_blend[2] = BlendDesc::to_factor(value);
					break;
				case constfn_hash("dst_blend_3"):
					bs_desc.dst_blend[3] = BlendDesc::to_factor(value);
					break;
				case constfn_hash("dst_blend_4"):
					bs_desc.dst_blend[4] = BlendDesc::to_factor(value);
					break;
				case constfn_hash("dst_blend_5"):
					bs_desc.dst_blend[5] = BlendDesc::to_factor(value);
					break;
				case constfn_hash("dst_blend_6"):
					bs_desc.dst_blend[6] = BlendDesc::to_factor(value);
				case constfn_hash("dst_blend_7"):
					bs_desc.dst_blend[7] = BlendDesc::to_factor(value);
					break;

				case constfn_hash("blend_op_alpha_0"):
					bs_desc.blend_op_alpha[0] = BlendDesc::to_op(value);
					break;
				case constfn_hash("blend_op_alpha_1"):
					bs_desc.blend_op_alpha[1] = BlendDesc::to_op(value);
					break;
				case constfn_hash("blend_op_alpha_2"):
					bs_desc.blend_op_alpha[2] = BlendDesc::to_op(value);
					break;
				case constfn_hash("blend_op_alpha_3"):
					bs_desc.blend_op_alpha[3] = BlendDesc::to_op(value);
					break;
				case constfn_hash("blend_op_alpha_4"):
					bs_desc.blend_op_alpha[4] = BlendDesc::to_op(value);
					break;
				case constfn_hash("blend_op_alpha_5"):
					bs_desc.blend_op_alpha[5] = BlendDesc::to_op(value);
					break;
				case constfn_hash("blend_op_alpha_6"):
					bs_desc.blend_op_alpha[6] = BlendDesc::to_op(value);
				case constfn_hash("blend_op_alpha_7"):
					bs_desc.blend_op_alpha[7] = BlendDesc::to_op(value);
					break;

				case constfn_hash("src_blend_alpha_0"):
					bs_desc.src_blend_alpha[0] = BlendDesc::to_factor(value);
					break;
				case constfn_hash("src_blend_alpha_1"):
					bs_desc.src_blend_alpha[1] = BlendDesc::to_factor(value);
					break;
				case constfn_hash("src_blend_alpha_2"):
					bs_desc.src_blend_alpha[2] = BlendDesc::to_factor(value);
					break;
				case constfn_hash("src_blend_alpha_3"):
					bs_desc.src_blend_alpha[3] = BlendDesc::to_factor(value);
					break;
				case constfn_hash("src_blend_alpha_4"):
					bs_desc.src_blend_alpha[4] = BlendDesc::to_factor(value);
					break;
				case constfn_hash("src_blend_alpha_5"):
					bs_desc.src_blend_alpha[5] = BlendDesc::to_factor(value);
					break;
				case constfn_hash("src_blend_alpha_6"):
					bs_desc.src_blend_alpha[6] = BlendDesc::to_factor(value);
				case constfn_hash("src_blend_alpha_7"):
					bs_desc.src_blend_alpha[7] = BlendDesc::to_factor(value);
					break;
				case constfn_hash("dst_blend_alpha_0"):
					bs_desc.dst_blend_alpha[0] = BlendDesc::to_factor(value);
					break;
				case constfn_hash("dst_blend_alpha_1"):
					bs_desc.dst_blend_alpha[1] = BlendDesc::to_factor(value);
					break;
				case constfn_hash("dst_blend_alpha_2"):
					bs_desc.dst_blend_alpha[2] = BlendDesc::to_factor(value);
					break;
				case constfn_hash("dst_blend_alpha_3"):
					bs_desc.dst_blend_alpha[3] = BlendDesc::to_factor(value);
					break;
				case constfn_hash("dst_blend_alpha_4"):
					bs_desc.dst_blend_alpha[4] = BlendDesc::to_factor(value);
					break;
				case constfn_hash("dst_blend_alpha_5"):
					bs_desc.dst_blend_alpha[5] = BlendDesc::to_factor(value);
					break;
				case constfn_hash("dst_blend_alpha_6"):
					bs_desc.dst_blend_alpha[6] = BlendDesc::to_factor(value);
				case constfn_hash("dst_blend_alpha_7"):
					bs_desc.dst_blend_alpha[7] = BlendDesc::to_factor(value);
					break;

				case constfn_hash("color_write_mask_0"):
					bs_desc.color_write_mask[0] = to_uint8(value);
					break;
				case constfn_hash("color_write_mask_1"):
					bs_desc.color_write_mask[1] = to_uint8(value);
					break;
				case constfn_hash("color_write_mask_2"):
					bs_desc.color_write_mask[2] = to_uint8(value);
					break;
				case constfn_hash("color_write_mask_3"):
					bs_desc.color_write_mask[3] = to_uint8(value);
					break;
				case constfn_hash("color_write_mask_4"):
					bs_desc.color_write_mask[4] = to_uint8(value);
					break;
				case constfn_hash("color_write_mask_5"):
					bs_desc.color_write_mask[5] = to_uint8(value);
					break;
				case constfn_hash("color_write_mask_6"):
					bs_desc.color_write_mask[7] = to_uint8(value);
					break;
				case constfn_hash("color_write_mask_7"):
					bs_desc.color_write_mask[7] = to_uint8(value);
					break;
				}
			};
			for (auto & child_node : pass_node) {
				try {
					auto first = leo::Access<std::string>(*child_node.begin());
					auto second = leo::Access<std::string>(*child_node.rbegin());
					auto first_hash = leo::constfn_hash(first.c_str());

					switch (first_hash)
					{
					case constfn_hash("rs_mode"):
						rs_desc.mode = RasterizerDesc::to_mode<RasterizerMode>(second);
						break;
					case constfn_hash("cull"):
						rs_desc.cull = RasterizerDesc::to_mode<CullMode>(second);
						break;
					case constfn_hash("ccw"):
						rs_desc.ccw = to_bool(second);
						break;
					case constfn_hash("depth_clip_enable"):
						rs_desc.depth_clip_enable = to_bool(second);
						break;
					case constfn_hash("scissor_enable"):
						rs_desc.scissor_enable = to_bool(second);
						break;
					case constfn_hash("multisample_enable"):
						rs_desc.multisample_enable = to_bool(second);
						break;

					case constfn_hash("depth_enable"):
						ds_desc.depth_enable = to_bool(second);
						break;
					case constfn_hash("depth_write_mask"):
						ds_desc.depth_enable = to_bool(second);
						break;
					case constfn_hash("depth_func"):
						ds_desc.depth_func = DepthStencilDesc::to_op<CompareOp>(second);
						break;

					case constfn_hash("front_stencil_enable"):
						ds_desc.front_stencil_enable = to_bool(second);
						break;
					case constfn_hash("front_stencil_ref"):
						ds_desc.front_stencil_ref = to_uint16(second);
						break;
					case constfn_hash("front_stencil_read_mask"):
						ds_desc.front_stencil_read_mask = to_uint16(second);
						break;
					case constfn_hash("front_stencil_write_mask"):
						ds_desc.front_stencil_write_mask = to_uint16(second);
						break;
					case constfn_hash("front_stencil_fail"):
						ds_desc.front_stencil_fail = DepthStencilDesc::to_op<StencilOp>(second);
						break;
					case constfn_hash("front_stencil_depth_fail"):
						ds_desc.front_stencil_depth_fail = DepthStencilDesc::to_op<StencilOp>(second);
						break;
					case constfn_hash("front_stencil_pass"):
						ds_desc.front_stencil_pass = DepthStencilDesc::to_op<StencilOp>(second);
						break;

					case constfn_hash("back_stencil_enable"):
						ds_desc.back_stencil_enable = to_bool(second);
						break;
					case constfn_hash("back_stencil_ref"):
						ds_desc.back_stencil_ref = to_uint16(second);
						break;
					case constfn_hash("back_stencil_read_mask"):
						ds_desc.back_stencil_read_mask = to_uint16(second);
						break;
					case constfn_hash("back_stencil_write_mask"):
						ds_desc.back_stencil_write_mask = to_uint16(second);
						break;
					case constfn_hash("back_stencil_fail"):
						ds_desc.back_stencil_fail = DepthStencilDesc::to_op<StencilOp>(second);
						break;
					case constfn_hash("back_stencil_depth_fail"):
						ds_desc.back_stencil_depth_fail = DepthStencilDesc::to_op<StencilOp>(second);
						break;
					case constfn_hash("back_stencil_pass"):
						ds_desc.back_stencil_pass = DepthStencilDesc::to_op<StencilOp>(second);
						break;

					case constfn_hash("blend_factor"):
						auto value = to_float4(second);
						bs_desc.blend_factor = M::Color(value.data);
						break;
					case constfn_hash("sample_mask"):
						bs_desc.sample_mask = to_uint32(second);
						break;
					case constfn_hash("alpha_to_coverage_enable"):
						bs_desc.alpha_to_coverage_enable = to_bool(second);
						break;
					case constfn_hash("independent_blend_enable"):
						bs_desc.independent_blend_enable = to_bool(second);
						break;
					default:
						default_parse(first_hash, second);
					}
				}
				CatchIgnore(leo::bad_any_cast &)
			}
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
