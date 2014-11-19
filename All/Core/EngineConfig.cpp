#include "EngineConfig.h"
#include "FileSearch.h"
#include "..\DeviceMgr.h"
#include "..\scheme_helper.h"
#include "..\IndePlatform\string.hpp"
#include <fstream>
namespace leo{
	void EngineConfig::Read(const std::wstring& configScheme){
		auto config_sexp = leo::parse_file(configScheme);

		auto window_sexp = leo::scheme::sexp::ops::find_sexp("window", config_sexp);

		auto width = static_cast<leo::uint16>(leo::expack_long(window_sexp, S("width")));
		auto height = static_cast<leo::uint16>(leo::expack_long(window_sexp, S("height")));

		global::globalClientSize.first = width;
		global::globalClientSize.second = height;

		auto dirs_sexp = leo::scheme::sexp::ops::find_sexp("search-dirs", config_sexp);
		auto dirs_num = leo::scheme::sexp::ops::sexp_list_length(dirs_sexp);

		auto dirs_iter = dirs_sexp->mNext;
		while (dirs_iter)
		{
			auto dir_sexp = leo::scheme::sexp::ops::find_sexp("dir", dirs_iter);
			FileSearch::PushSearchDir(towstring(dir_sexp->mNext->mValue.cast_atom<scheme::sexp::sexp_string>()));
			dirs_iter = dirs_iter->mNext;
		}
	}
	void EngineConfig::Write(const std::wstring& configScheme){
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
		for (auto & dir : SearchDirectors()){
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

	const std::pair<uint16, uint16>& EngineConfig::ClientSize(){
		return global::globalClientSize;
	}
	const std::vector<std::wstring>& EngineConfig::SearchDirectors(){
		return FileSearch::SearchDirectors();

	}
}