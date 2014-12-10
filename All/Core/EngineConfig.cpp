#include "..\IndePlatform\platform.h"
#include "..\IndePlatform\string.hpp"

#include "EngineConfig.h"
#include "FileSearch.h"

#include "..\d3dx11.hpp"
#include "..\exception.hpp"
#include "..\DeviceMgr.h"
#include "..\scheme_helper.h"

#include <fstream>
namespace leo {

	static leo::scheme::sexp::sexp_list read_config_sexp = nullptr;

	static scheme::sexp::sexp_list pack_effect(const std::wstring& shader);

	void EngineConfig::Read(const std::wstring& configScheme) {
		read_config_sexp = leo::parse_file(configScheme);

		auto window_sexp = leo::scheme::sexp::ops::find_sexp("window", read_config_sexp);

		auto width = static_cast<leo::uint16>(leo::expack_long(window_sexp, S("width")));
		auto height = static_cast<leo::uint16>(leo::expack_long(window_sexp, S("height")));

		global::globalClientSize.first = width;
		global::globalClientSize.second = height;

		auto dirs_sexp = leo::scheme::sexp::ops::find_sexp("search-dirs", read_config_sexp);
		auto dirs_num = leo::scheme::sexp::ops::sexp_list_length(dirs_sexp)-1;

		auto dirs_iter = dirs_sexp->mNext;
		while (dirs_iter)
		{
			auto dir_sexp = leo::scheme::sexp::ops::find_sexp("dir", dirs_iter);
			FileSearch::PushSearchDir(to_wstring(dir_sexp->mNext->mValue.cast_atom<scheme::sexp::sexp_string>()));
			dirs_iter = dirs_iter->mNext;
		}
	}
	void EngineConfig::Write(const std::wstring& configScheme) {
		using namespace scheme;
		auto config_sexp = sexp::make_sexp_word(S("config"));

		auto window_sexp = sexp::make_sexp_word(S("window"));
		auto width_sexp = pack_key_value(S("width"), ClientSize().first);
		auto height_sexp = pack_key_value(S("height"), ClientSize().second);
		window_sexp->mNext = sexp::make_sexp(sexp::sexp_list(width_sexp));
		window_sexp->mNext->mNext = sexp::make_sexp(sexp::sexp_list(height_sexp));



		config_sexp->mNext = sexp::make_sexp(sexp::sexp_list(window_sexp));

		auto dirs_sexp = sexp::make_sexp_word(S("search-dirs"));
		auto prev = dirs_sexp;
		for (auto & dir : SearchDirectors()) {
			auto dir_sexp = pack_key_value(S("dir"), to_string(dir));
			prev->mNext = sexp::make_sexp(sexp::sexp_list(dir_sexp));
			prev = prev->mNext;
		}
		config_sexp->mNext->mNext = sexp::make_sexp(std::shared_ptr<sexp::sexp>(dirs_sexp));

		auto effects_sexp = sexp::make_sexp_word(S("effects"));
		prev = effects_sexp;
		for (auto & shader : ShaderConfig::GetAllShaderName()) {
			auto effect_sexp = pack_effect(shader);
			prev->mNext = sexp::make_sexp(sexp::sexp_list(effect_sexp));
			prev = prev->mNext;
		}

		//end
		//config_sexp->mNext->mNext->mNext = effects_sexp;
		config_sexp->mNext->mNext->mNext = sexp::make_sexp(std::shared_ptr<sexp::sexp>(effects_sexp));

		auto config_string = scheme::sexp::ops::print_sexp(config_sexp);

		std::ofstream fout(configScheme);
		const unsigned char BOM[] = { 0xEFu, 0XBBu,0xBFu };
		fout.write((char*)BOM, 3);
		fout.write(config_string.c_str(), config_string.size());
	}

	const std::pair<uint16, uint16>& EngineConfig::ClientSize() {
		return global::globalClientSize;
	}
	const std::vector<std::wstring>& EngineConfig::SearchDirectors() {
		return FileSearch::SearchDirectors();

	}

	struct ShaderFileName {
		std::wstring mFileName[6];
	};

	static std::vector<ShaderFileName> mShaderFileNames;
	static std::vector<std::wstring> mShaderNames;
	static std::vector<std::wstring> mSamplNames;
	static std::vector<std::wstring> mDepthNames;
	static std::vector<std::wstring> mBlendNames;
	static std::vector<std::wstring> mRasteNames;

	static std::vector<D3D11_RASTERIZER_DESC> mRasteDescs;
	static std::vector<D3D11_SAMPLER_DESC> mSamplDescs;
	static std::vector<D3D11_DEPTH_STENCIL_DESC> mDepthDescs;
	static std::vector<D3D11_BLEND_DESC> mBlendDescs;

	static void expack_effect(scheme::sexp::sexp_list effect_sexp) {
		using namespace scheme;
		auto name_sexp = sexp::ops::find_sexp("name", effect_sexp);
		mShaderNames.push_back(
				to_wstring(
				name_sexp->mNext->mValue.cast_atom<sexp::sexp_string>()
				)
			);
		auto index = mShaderFileNames.size();
		mShaderFileNames.push_back(ShaderFileName());
		auto shader_sexp = sexp::ops::find_sexp("shader", effect_sexp);
		auto shader_iter = shader_sexp->mNext;
		while (shader_iter)
		{
			auto value_sexp = shader_iter->mValue.cast_list();
			auto type = value_sexp->mValue.cast_atom<sexp::sexp_string>()[0];
			auto file =to_wstring(value_sexp->mNext->mValue.cast_atom <sexp::sexp_string>());
			switch (type)
			{
			case 'v':
			case 'V':
				mShaderFileNames[index].mFileName[D3D11_VERTEX_SHADER - 1] = file;
				break;
			case 'p':
			case 'P':
				mShaderFileNames[index].mFileName[D3D11_PIXEL_SHADER - 1] = file;
				break;
			case 'g':
			case 'G':
				mShaderFileNames[index].mFileName[D3D11_GEOMETRY_SHADER - 1] = file;
				break;
			case 'c':
			case 'C':
				mShaderFileNames[index].mFileName[D3D11_COMPUTE_SHADER - 1] = file;
				break;
			case 'h':
			case 'H':
				mShaderFileNames[index].mFileName[D3D11_HULL_SHADER - 1] = file;
				break;
			case 'd':
			case 'D':
				mShaderFileNames[index].mFileName[D3D11_DOMAIN_SHADER - 1] = file;
				break;
			default:
				Raise_Error_Exception(ERROR_INVALID_PARAMETER, "读取\"" + to_string(mSamplNames[index]) + " \"Shader出现了无法识别的Shader类型");
			}
			shader_iter = shader_iter->mNext;
		}
	}

	void pack_shader(const std::wstring& shader,scheme::sexp::sexp_list& prev, D3D11_SHADER_TYPE s) {
		using namespace scheme;
		if (!EngineConfig::ShaderConfig::GetShaderFileName(shader, s).empty()) {
			std::string type;
			switch (s)
			{
			case D3D11_VERTEX_SHADER:
				type = "v";
				break;
			case D3D11_HULL_SHADER:
				type = "h";
				break;
			case D3D11_DOMAIN_SHADER:
				type = "d";
				break;
			case D3D11_GEOMETRY_SHADER:
				type = "g";
				break;
			case D3D11_PIXEL_SHADER:
				type = "p";
				break;
			case D3D11_COMPUTE_SHADER:
				type = "c";
				break;
			default:
				break;
			}
			prev->mNext = sexp::make_sexp(
				sexp::sexp_list(pack_key_value(
				type,
				to_string(EngineConfig::ShaderConfig::GetShaderFileName(shader, s)))
				));
			prev = prev->mNext;
		}
	}

	scheme::sexp::sexp_list pack_effect(const std::wstring& shader) {
		using namespace scheme;
		auto effect_sexp = sexp::make_sexp_word(S("effect"));

		auto name_sexp = pack_key_value("name", to_string(shader));
		effect_sexp->mNext = sexp::make_sexp(sexp::sexp_list(name_sexp));

		auto shader_sexp = sexp::make_sexp_word(S("shader"));
		auto prev = shader_sexp;
		pack_shader(shader, prev, D3D11_VERTEX_SHADER);
		pack_shader(shader, prev, D3D11_PIXEL_SHADER);
		pack_shader(shader, prev, D3D11_GEOMETRY_SHADER);
		pack_shader(shader, prev, D3D11_HULL_SHADER);
		pack_shader(shader, prev, D3D11_DOMAIN_SHADER);
		pack_shader(shader, prev, D3D11_COMPUTE_SHADER);
		//end
		effect_sexp->mNext->mNext = sexp::make_sexp(shader_sexp);

		return effect_sexp;
	}
	static void ShaderConfigInit() {
		using namespace scheme;
		static bool init = false;
		if (!init) {

			auto effects_sexp = sexp::ops::find_sexp("effects", read_config_sexp);
			if (effects_sexp) {
				auto effects_num = sexp::ops::sexp_list_length(effects_sexp) - 1;

				auto effects_iter = effects_sexp->mNext;
				while (effects_iter)
				{
					auto effect_sexp = sexp::ops::find_sexp("effect", effects_iter);
					expack_effect(effect_sexp);

					effects_iter = effects_iter->mNext;
				}
			}

			init = true;
		}
	}


	template<typename Index,typename Contain>
	const typename Contain::value_type& find_helper(const Contain& vals, const Index& indexs, const typename Index::value_type& index) {
		auto it = std::find(std::begin(indexs), std::end(indexs), index);
		if(it == std::end(indexs))
			Raise_Error_Exception(ERROR_INVALID_PARAMETER, "要查找的"+to_string(index)+"不存在");
		return vals[std::distance(std::begin(indexs), it)];
	}

	const std::vector<std::wstring>& EngineConfig::ShaderConfig::GetAllShaderName() {
		ShaderConfigInit();
		return mShaderNames;
	}
	const std::wstring& EngineConfig::ShaderConfig::GetShaderFileName(const std::wstring& shaderName, D3D11_SHADER_TYPE shaderType) {
		ShaderConfigInit();
		return find_helper(mShaderFileNames, mShaderNames, shaderName).mFileName[shaderType - 1];
	}

	const std::vector<std::wstring>& EngineConfig::ShaderConfig::GetAllSampleStateName() {
		ShaderConfigInit();
		return mSamplNames;
	}
	const std::vector<std::wstring>& EngineConfig::ShaderConfig::GetAllDepthStencilStateName() {
		ShaderConfigInit();
		return mDepthNames;
	}
	const std::vector<std::wstring>& EngineConfig::ShaderConfig::GetAllRasterizerStateName() {
		ShaderConfigInit();
		return mRasteNames;
	}
	const std::vector<std::wstring>& EngineConfig::ShaderConfig::GetAllBlendStateName() {
		ShaderConfigInit();
		return mBlendNames;
	}

	const D3D11_RASTERIZER_DESC& EngineConfig::ShaderConfig::GetRasterizerState(const std::wstring& rasName) {
		ShaderConfigInit();
		return find_helper(mRasteDescs, mRasteNames, rasName);
	}
	const D3D11_DEPTH_STENCIL_DESC& EngineConfig::ShaderConfig::GetDepthStencilState(const std::wstring& depName) {
		ShaderConfigInit();
		return find_helper(mDepthDescs, mDepthNames, depName);
	}
	const D3D11_BLEND_DESC& EngineConfig::ShaderConfig::GetBlendState(const std::wstring& bleName) {
		ShaderConfigInit();
		return find_helper(mBlendDescs, mBlendNames, bleName);
	}
	const D3D11_SAMPLER_DESC& EngineConfig::ShaderConfig::GetSampleState(const std::wstring& samName) {
		ShaderConfigInit();
		return find_helper(mSamplDescs, mSamplNames, samName);
	}


}