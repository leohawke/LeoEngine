#include<LBase/typeinfo.h>
#include <LBase/id.hpp>

#include <LScheme/LScheme.h>

#include "LFramework/Win32/LCLib/COM.h"
#include "LFramework/Helper/ShellHelper.h"

#include "../Render/IContext.h"
#include "../Core/AssetResourceScheduler.h"

#include "EffectX.h"
#include "LSLAssetX.h"

#include <memory>
#include <fstream>
#include <iterator>


#pragma warning(disable:4715) //return value or throw exception;

namespace D3DFlags {
	enum COMPILER_FLAGS
	{
		D3DCOMPILE_DEBUG = (1 << 0),
		D3DCOMPILE_SKIP_VALIDATION = (1 << 1),
		D3DCOMPILE_SKIP_OPTIMIZATION = (1 << 2),
		D3DCOMPILE_PACK_MATRIX_ROW_MAJOR = (1 << 3),
		D3DCOMPILE_PACK_MATRIX_COLUMN_MAJOR = (1 << 4),
		D3DCOMPILE_PARTIAL_PRECISION = (1 << 5),
		D3DCOMPILE_FORCE_VS_SOFTWARE_NO_OPT = (1 << 6),
		D3DCOMPILE_FORCE_PS_SOFTWARE_NO_OPT = (1 << 7),
		D3DCOMPILE_NO_PRESHADER = (1 << 8),
		D3DCOMPILE_AVOID_FLOW_CONTROL = (1 << 9),
		D3DCOMPILE_PREFER_FLOW_CONTROL = (1 << 10),
		D3DCOMPILE_ENABLE_STRICTNESS = (1 << 11),
		D3DCOMPILE_ENABLE_BACKWARDS_COMPATIBILITY = (1 << 12),
		D3DCOMPILE_IEEE_STRICTNESS = (1 << 13),
		D3DCOMPILE_OPTIMIZATION_LEVEL0 = (1 << 14),
		D3DCOMPILE_OPTIMIZATION_LEVEL1 = 0,
		D3DCOMPILE_OPTIMIZATION_LEVEL2 = ((1 << 14) | (1 << 15)),
		D3DCOMPILE_OPTIMIZATION_LEVEL3 = (1 << 15),
		D3DCOMPILE_RESERVED16 = (1 << 16),
		D3DCOMPILE_RESERVED17 = (1 << 17),
		D3DCOMPILE_WARNINGS_ARE_ERRORS = (1 << 18),
		D3DCOMPILE_RESOURCES_MAY_ALIAS = (1 << 19),
		D3DCOMPILE_ENABLE_UNBOUNDED_DESCRIPTOR_TABLES = (1 << 20),
		D3DCOMPILE_ALL_RESOURCES_BOUND = (1 << 21),
	};

	enum COMPILER_STRIP_FLAGS
	{
		D3DCOMPILER_STRIP_REFLECTION_DATA = 0x00000001,
		D3DCOMPILER_STRIP_DEBUG_INFO = 0x00000002,
		D3DCOMPILER_STRIP_TEST_BLOBS = 0x00000004,
		D3DCOMPILER_STRIP_PRIVATE_DATA = 0x00000008,
		D3DCOMPILER_STRIP_ROOT_SIGNATURE = 0x00000010,
		D3DCOMPILER_STRIP_FORCE_DWORD = 0x7fffffff,
	};
}


namespace platform {
	using namespace scheme;

	std::vector<asset::EffectMacro> AppendCompileMacros(const std::vector<asset::EffectMacro>& macros, asset::ShaderBlobAsset::Type type);
	std::string_view CompileProfile(asset::ShaderBlobAsset::Type type);

	class EffectLoadingDesc : public asset::AssetLoading<asset::EffectAsset> {
	private:
		struct EffectDesc {
			X::path effect_path;
			struct Data {
				scheme::TermNode effect_node;
			};
			std::shared_ptr<Data> data;
			std::shared_ptr<AssetType> effect_asset;
			std::string effect_code;
		} effect_desc;
	public:
		explicit EffectLoadingDesc(X::path const & effectpath)
		{
			effect_desc.effect_path = effectpath;
		}

		std::size_t Type() const override {
			return leo::type_id<EffectLoadingDesc>().hash_code();
		}

		std::size_t Hash() const override {
			return leo::hash_combine_seq(Type(), effect_desc.effect_path.wstring());
		}

		const asset::path& Path() const override {
			return effect_desc.effect_path;
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
			effect_desc.effect_asset->SetName(effect_desc.effect_path.string());
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
				auto cbuffer_nodes = X::SelectNodes("cbuffer", effect_node);
				for (auto & cbuffer_node : cbuffer_nodes) {
					asset::EffectConstantBufferAsset cbuffer;
					cbuffer.SetName(AccessLastNoChild<std::string>(cbuffer_node));
					auto param_nodes = cbuffer_node.SelectChildren([&](const scheme::TermNode& child) {
						if (child.empty())
							return false;
						try {
							return  AssetType::GetType(leo::Access<std::string>(*child.begin())) != asset::EPT_shader;
						}
						catch (leo::unsupported&) {
							return false;
						}
					});
					std::vector<leo::uint32> ParamIndices;
					for (auto & param_node : param_nodes) {
						auto param_index = ParseParam(param_node);
						ParamIndices.emplace_back(static_cast<leo::uint32>(param_index));
					}
					cbuffer.GetParamIndicesRef() = std::move(ParamIndices);
					effect_desc.effect_asset->GetCBuffersRef().emplace_back(std::move(cbuffer));
				}
			}
			//parser params
			{
				auto param_nodes = effect_node.SelectChildren([&](const scheme::TermNode& child) {
					if (child.empty())
						return false;
					try {
						return  AssetType::GetType(leo::Access<std::string>(*child.begin())) != asset::EPT_shader;
					}
					catch (leo::unsupported&) {
						return false;
					}
				});
				for (auto & param_node : param_nodes)
					ParseParam(param_node);
			}
			{
				auto fragments = X::SelectNodes("shader", effect_node);
				for (auto& fragment : fragments) {
					effect_desc.effect_asset->GetFragmentsRef().emplace_back();
					effect_desc.effect_asset->GetFragmentsRef().back().
						GetFragmentRef() = scheme::Deliteralize(
							leo::Access<std::string>(*fragment.rbegin())
						);
				}
			}
			effect_desc.effect_code = effect_desc.effect_asset->GenHLSLShader();
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

		template<typename _type,typename _func>
		const _type& ReverseAccess(_func f, const scheme::TermNode& node) {
			auto it = std::find_if(node.rbegin(), node.rend(),f);
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

		void ParseMacro(std::vector<asset::EffectMacro>& macros, const scheme::TermNode& node)
		{
			//macro (macro (name foo) (value bar))
			auto macro_nodes = X::SelectNodes("macro", node);
			for (auto & macro_node : macro_nodes) {
				macros.emplace_back(
					Access("name", macro_node),
					Access("value", macro_node));
			}
			UniqueMacro(macros);
		}

		void ParseTechnique(const scheme::TermNode& effect_node)
		{
			auto techniques = X::SelectNodes("technique", effect_node);
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

				auto passes = X::SelectNodes("pass", technique_node);
				for (auto & pass_node : passes) {
					asset::TechniquePassAsset pass;
					pass.SetName(Access("name", pass_node));

					asset::TechniquePassAsset* pass_ptr = nullptr;
					if (inherit) {
						auto& exist_passes = technique.GetPassesRef();
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
					ComposePassShader(technique, *pass_ptr, pass_node);
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

			

			auto to_uint16 = [](const std::string& value) {
				return static_cast<leo::uint16>(std::stoul(value));
			};

			auto to_uint32 = [](const std::string& value) {
				return static_cast<leo::uint32>(std::stoul(value));
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
				if (child_node.size() != 2)
					continue;
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
						ds_desc.depth_write_mask = to_bool(second);
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

		void ComposePassShader(const asset::EffectTechniqueAsset&technique, asset::TechniquePassAsset& pass, scheme::TermNode& pass_node) {
			size_t seed = 0;
			auto macro_hash = leo::combined_hash<asset::EffectMacro>();
			std::vector<asset::EffectMacro> macros{ effect_desc.effect_asset->GetMacros() };
			for (auto & macro_pair : technique.GetMacros()) {
				leo::hash_combine(seed, macro_hash(macro_pair));
				macros.emplace_back(macro_pair);
			}
			for (auto & macro_pair : pass.GetMacros()) {
				leo::hash_combine(seed, macro_hash(macro_pair));
				macros.emplace_back(macro_pair);
			}

			for (auto & child_node : pass_node) {
				if (child_node.size() != 2)
					continue;
				try {
					auto first = leo::Access<std::string>(*child_node.begin());
					auto second = leo::Access<std::string>(*child_node.rbegin());
					auto first_hash = leo::constfn_hash(first);

					asset::ShaderBlobAsset::Type compile_type;
					string_view compile_entry_point = second;
					size_t blob_hash = seed;
					leo::hash_combine(blob_hash,leo::constfn_hash(second));

					switch (first_hash) {
					case leo::constfn_hash("vertex_shader"):
						compile_type = platform::Render::ShaderCompose::Type::VertexShader;
						break;
					case leo::constfn_hash("pixel_shader"):
						compile_type = platform::Render::ShaderCompose::Type::PixelShader;
						break;
					default:
						continue;
					}
					string_view profile = CompileProfile(compile_type);
					using namespace leo;
					LFL_DEBUG_DECL_TIMER(ComposePassShader,sfmt("CompilerReflectStrip Type:%s ", first.c_str()))
					auto blob = X::Shader::CompileToDXBC(compile_type, effect_desc.effect_code, compile_entry_point, AppendCompileMacros(macros, compile_type), profile,
						D3DFlags::D3DCOMPILE_ENABLE_STRICTNESS |
#ifndef NDEBUG
						D3DFlags::D3DCOMPILE_DEBUG
#else
						D3DFlags::D3DCOMPILE_OPTIMIZATION_LEVEL3
#endif
						, effect_desc.effect_path.string()
					);
					auto pInfo = leo::unique_raw(X::Shader::ReflectDXBC(blob, compile_type));
					blob.swap(X::Shader::StripDXBC(blob, D3DFlags::D3DCOMPILER_STRIP_DEBUG_INFO
						| D3DFlags::D3DCOMPILER_STRIP_PRIVATE_DATA));

					effect_desc.effect_asset->EmplaceBlob(blob_hash, asset::ShaderBlobAsset(compile_type, std::move(blob),pInfo.release()));
					pass.AssignOrInsertHash(compile_type, blob_hash);
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

		size_t ParseParam(const scheme::TermNode& param_node) {
			asset::EffectParameterAsset param;
			param.SetName(AccessLastNoChild<std::string>(param_node));
			//don't need check type again
			param.GetTypeRef() = AssetType::GetType(leo::Access<std::string>(*param_node.begin()));
			if (param.GetType() >= asset::EPT_bool) {
				if (auto p = leo::AccessChildPtr<std::string>(param_node, "arraysize"))
					param.GetArraySizeRef() = std::stoul(*p);
			}
			else if (param.GetType() <= asset::EPT_ConsumeStructuredBuffer) {
				if (auto elemtype = AccessPtr("elemtype", param_node))
					param.GetElemTypeRef() = AssetType::GetType(*elemtype);
			}
			auto index = effect_desc.effect_asset->GetParams().size();
			auto optional_value = ReadParamValue(param_node,param.GetType());
			if (optional_value.has_value())
				effect_desc.effect_asset->BindValue(index, optional_value.value());
			effect_desc.effect_asset->GetParamsRef().emplace_back(std::move(param));
			return index;
		}

		static leo::math::float4 to_float4 (const std::string& value) {
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
		
		std::optional<leo::any> ReadParamValue(const scheme::TermNode& param_node,asset::EffectParamType type) {
#define AccessParam(name,expr) if (auto value = AccessPtr(name, param_node)) expr
			using namespace Render;
			if (type == asset::EPT_sampler) {
				SamplerDesc sampler;
				AccessParam("border_clr", sampler.border_clr = M::Color(to_float4(*value).data));

				AccessParam("address_mode_u",sampler.address_mode_u = SamplerDesc::to_mode(*value));
				AccessParam("address_mode_v", sampler.address_mode_v = SamplerDesc::to_mode(*value));
				AccessParam("address_mode_w", sampler.address_mode_w = SamplerDesc::to_mode(*value));

				AccessParam("filtering", sampler.filtering = SamplerDesc::to_op<TexFilterOp>(*value));

				AccessParam("max_anisotropy", sampler.max_anisotropy = to_uint8(*value));

				AccessParam("min_lod", sampler.min_lod = std::stof(*value));
				AccessParam("max_lod", sampler.max_lod = std::stof(*value));
				AccessParam("mip_map_lod_bias", sampler.mip_map_lod_bias = std::stof(*value));

				AccessParam("cmp_func", sampler.cmp_func = SamplerDesc::to_op<CompareOp>(*value));

				return leo::any(sampler);
			}
#undef AccessParam
			return std::nullopt;
		}
	};

	std::shared_ptr<asset::EffectAsset> X::LoadEffectAsset(path const & effectpath)
	{
		return  AssetResourceScheduler::Instance().SyncLoad<EffectLoadingDesc>(effectpath);
	}
	std::shared_ptr<Render::Effect::Effect> platform::X::LoadEffect(std::string const & name)
	{
		return  AssetResourceScheduler::Instance().SyncSpawnResource<Render::Effect::Effect>(name);
	}
}

namespace platform {
	std::vector<asset::EffectMacro> AppendCompileMacros(const std::vector<asset::EffectMacro>& macros, asset::ShaderBlobAsset::Type type)
	{
		using namespace  platform::Render;

		auto append_macros = macros;
		auto caps = Context::Instance().GetDevice().GetCaps();
		switch (caps.type) {
		case Caps::Type::D3D12:
			append_macros.emplace_back("D3D12", "1");
			//TODO depend feature_level
			append_macros.emplace_back("SM_VERSION", "50");
			break;
		}
		switch (type) {
		case ShaderCompose::Type::VertexShader:
			append_macros.emplace_back("VS", "1");
			break;
		case ShaderCompose::Type::PixelShader:
			append_macros.emplace_back("PS", "1");
			break;
		}
		return append_macros;
	}

	std::string_view CompileProfile(asset::ShaderBlobAsset::Type type)
	{
		using namespace  platform::Render;

		auto caps = Context::Instance().GetDevice().GetCaps();
		switch (caps.type) {
		case Caps::Type::D3D12:
			switch (type) {
			case ShaderCompose::Type::VertexShader:
				return "vs_5_0";
			case ShaderCompose::Type::PixelShader:
				return "ps_5_0";
			}
		}
		return "";
	}
}


#include <LFramework/LCLib/Platform.h>

#ifdef LFL_Win32

namespace platform_ex::Windows::D3D12 {
	platform::Render::ShaderInfo * ReflectDXBC(const platform::Render::ShaderCompose::ShaderBlob & blob, platform::Render::ShaderCompose::Type type);
}

#include <UniversalDXSDK/d3dcompiler.h>
namespace platform::X::Shader {
	Render::ShaderCompose::ShaderBlob CompileToDXBC(Render::ShaderCompose::Type type, std::string_view code,
		std::string_view entry_point, const std::vector<asset::EffectMacro>& macros,
		std::string_view profile, leo::uint32 flags, string_view SourceName) {
		std::vector<D3D_SHADER_MACRO> defines;
		for (auto& macro : macros) {
			D3D_SHADER_MACRO define;
			define.Name = macro.first.c_str();
			define.Definition = macro.second.c_str();
			defines.emplace_back(define);
		}
		D3D_SHADER_MACRO define_end = { nullptr, nullptr };
		defines.push_back(define_end);

		platform_ex::COMPtr<ID3DBlob> code_blob;
		platform_ex::COMPtr<ID3DBlob> error_blob;

		auto hr = D3DCompile(code.data(), code.size(), SourceName.data(), defines.data(), nullptr, entry_point.data(), profile.data(), flags, 0, &code_blob, &error_blob);
		if (code_blob) {
			Render::ShaderCompose::ShaderBlob blob;
			blob.first = std::make_unique<stdex::byte[]>(code_blob->GetBufferSize());
			blob.second = code_blob->GetBufferSize();
			std::memcpy(blob.first.get(), code_blob->GetBufferPointer(), blob.second);
			return std::move(blob);
		}
		//TODO error_blob
		auto error = reinterpret_cast<char*>(error_blob->GetBufferPointer());
		LE_LogError(error);
		platform_ex::CheckHResult(hr);
	}

	Render::ShaderInfo * ReflectDXBC(const Render::ShaderCompose::ShaderBlob & blob, Render::ShaderCompose::Type type)
	{
		using namespace Render;
		auto caps = Context::Instance().GetDevice().GetCaps();
		switch (caps.type) {
		case Caps::Type::D3D12:
			return platform_ex::Windows::D3D12::ReflectDXBC(blob, type);
		}
	}
	
	Render::ShaderCompose::ShaderBlob StripDXBC(const Render::ShaderCompose::ShaderBlob& code_blob, leo::uint32 flags) {
		platform_ex::COMPtr<ID3DBlob> stripped_blob;
		platform_ex::CheckHResult(D3DStripShader(code_blob.first.get(), code_blob.second, flags, &stripped_blob));
		Render::ShaderCompose::ShaderBlob blob;
		blob.first = std::make_unique<stdex::byte[]>(stripped_blob->GetBufferSize());
		blob.second = stripped_blob->GetBufferSize();
		std::memcpy(blob.first.get(), stripped_blob->GetBufferPointer(), blob.second);
		return std::move(blob);
	}
}

#else
//TODO CryEngine HLSLCross Compiler?
//Other Target Platfom Compiler [Tool...]
#endif
