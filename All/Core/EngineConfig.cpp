#include "EngineConfig.h"
#include "FileSearch.h"
#include "..\DeviceMgr.h"
#include "..\scheme_helper.h"

namespace leo{
	void EngineConfig::Read(const std::wstring& configScheme){
		auto config_sexp = leo::parse_file(configScheme);

		auto window_sexp = leo::scheme::sexp::find_sexp(L"window", config_sexp);

		auto width = static_cast<leo::uint16>(leo::expack_long(window_sexp, S("width")));
		auto height = static_cast<leo::uint16>(leo::expack_long(window_sexp, S("height")));

		global::globalClientSize.first = width;
		global::globalClientSize.second = height;

		auto dirs_sexp = leo::scheme::sexp::find_sexp(L"search-dirs", config_sexp);
		auto dirs_num = leo::scheme::sexp::sexp_list_length(dirs_sexp);

		auto dirs_iter = dirs_sexp->next;
		while (dirs_iter)
		{
			auto dir_sexp = leo::scheme::sexp::find_sexp(L"dir", dirs_iter);
			dir_sexp->next->value;
			dirs_iter = dirs_iter->next;
		}
	}
	void EngineConfig::Write(const std::wstring& configScheme){
		using namespace scheme;
		auto config_sexp = leo::scheme::sexp::make_sexp_atom(S("config"),sexp::sexp::atom_string);

		auto window_sexp = leo::scheme::sexp::make_sexp_atom(S("window"), sexp::sexp::atom_string);
		auto width_sexp = pack_key_value(S("width"), global::globalClientSize.first);
		auto height_sexp = pack_key_value(S("height"), global::globalClientSize.second);
		window_sexp->next = sexp::make_sexp_list(std::shared_ptr<sexp::sexp>(width_sexp));
		window_sexp->next->next = height_sexp;



		config_sexp->next =sexp::make_sexp_list(std::shared_ptr<sexp::sexp>(window_sexp));

		auto dirs_sexp = sexp::make_sexp_atom(S("search-dirs"), sexp::sexp::atom_string);
		//Todo Insert window




		config_sexp->next->next = sexp::make_sexp_list(std::shared_ptr<sexp::sexp>(dirs_sexp));
	}

	std::pair<uint16, uint16> EngineConfig::ClientSize(){
		return global::globalClientSize;
	}
	std::vector<std::wstring> EngineConfig::SearchDirectory(){

	}
}