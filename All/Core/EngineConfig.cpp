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

	void EngineConfig::Read(const std::wstring& configScheme) {
		read_config_sexp = leo::parse_file(configScheme);

		auto window_sexp = leo::scheme::sexp::ops::find_sexp("window", read_config_sexp);

		auto width = static_cast<leo::uint16>(leo::expack_long(window_sexp, S("width")));
		auto height = static_cast<leo::uint16>(leo::expack_long(window_sexp, S("height")));

		global::globalClientSize.first = width;
		global::globalClientSize.second = height;

		auto dirs_sexp = leo::scheme::sexp::ops::find_sexp("search-dirs", read_config_sexp);
		auto dirs_num = leo::scheme::sexp::ops::sexp_list_length(dirs_sexp);

		auto dirs_iter = dirs_sexp->mNext;
		while (dirs_iter)
		{
			auto dir_sexp = leo::scheme::sexp::ops::find_sexp("dir", dirs_iter);
			FileSearch::PushSearchDir(towstring(dir_sexp->mNext->mValue.cast_atom<scheme::sexp::sexp_string>()));
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
			auto & next = prev->mNext;
			auto dir_sexp = pack_key_value(S("dir"), tostring(dir));
			next = sexp::make_sexp(sexp::sexp_list(dir_sexp));
			prev = next->mNext;
		}



		//end
		//config_sexp->mNext->mNext = dirs_sexp;
		config_sexp->mNext->mNext = sexp::make_sexp(std::shared_ptr<sexp::sexp>(dirs_sexp));

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

	static std::vector<ShaderFileName> mShaderFileName;
	static std::vector<std::wstring> mShadeName;
	static std::vector<std::wstring> mSamplName;
	static std::vector<std::wstring> mDepthName;
	static std::vector<std::wstring> mBlendName;
	static std::vector<std::wstring> mRasteName;

	static void ShaderConfigInit() {
		static bool init = false;
		if (!init) {

		}
	}


	template<typename Index,typename Contain>
	const typename Contain::value_type& find_helper(const Contain& vals, const Index& indexs, const typename Index::value_type& index) {
		Raise_Error_Exception(ERROR_INVALID_PARAMETER, "要查找的"+to_string(index)+"不存在");
	}

	const std::vector<std::wstring>& EngineConfig::ShaderConfig::GetAllShaderName() {
		ShaderConfigInit();
		return mShadeName;
	}
	const std::wstring& EngineConfig::ShaderConfig::GetShaderFileName(const std::wstring&, D3D11_SHADER_TYPE) {
		ShaderConfigInit();
	}

	const std::vector<std::wstring>& EngineConfig::ShaderConfig::GetAllSampleStateName() {
		ShaderConfigInit();
	}
	const std::vector<std::wstring>& EngineConfig::ShaderConfig::GetAllDepthStencilStateName() {
		ShaderConfigInit();
	}
	const std::vector<std::wstring>& EngineConfig::ShaderConfig::GetAllRasterizerStateName() {
		ShaderConfigInit();
	}
	const std::vector<std::wstring>& EngineConfig::ShaderConfig::GetAllBlendStateName() {
		ShaderConfigInit();
	}

	const D3D11_RASTERIZER_DESC& EngineConfig::ShaderConfig::GetRasterizerState(const std::wstring&) {
		ShaderConfigInit();
	}
	const D3D11_DEPTH_STENCIL_DESC& EngineConfig::ShaderConfig::GetDepthStencilState(const std::wstring&) {
		ShaderConfigInit();
	}
	const D3D11_BLEND_DESC& EngineConfig::ShaderConfig::GetBlendState(const std::wstring&) {
		ShaderConfigInit();
	}
	const D3D11_SAMPLER_DESC& EngineConfig::ShaderConfig::GetSampleState(const std::wstring&) {
		ShaderConfigInit();
	}


}