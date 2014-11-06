#include "Sexp/sexp.hpp"
#include "scheme.h"
#include <windows.h>
#include <array>
#include <cassert>


using namespace leo::scheme;


std::shared_ptr<sexp::sexp> pack(sexp::sexp_char * tag, float v1, float v2, float v3)
{
	auto sx = sexp::make_sexp(sexp::sexp_string(tag));

	sx->mNext = sexp::make_sexp(v1);

	sx->mNext->mNext = sexp::make_sexp(v2);

	sx->mNext->mNext->mNext = sexp::make_sexp(v3);

	return sx;
}

int main()
{
	DWORD write;
	auto consoleout = CreateFileW(L"CONOUT$", GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	auto stdhandle = GetStdHandle(STD_OUTPUT_HANDLE);

	auto tokens = sexp::lexicalanalysis("(if 1 (if #t 2 4) 3)");
	auto parse_sexp = sexp::parse(tokens);

	auto parse_schem = make_copy(parse_sexp);

	auto result = eval(parse_schem, scheme_nil);
	auto a = result.cast_atom().cast<scheme_int>();
	//auto sexp = pack("tag", 1.0112345f, 2.f, 3.f);
#if 0
	auto scheme_env = make_scheme_list();
	for (;;){
		write(eval(read(), scheme_env));
	}
#endif
	return 0;
}
