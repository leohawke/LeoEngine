#include <IndePlatform\platform.h>
#include "Sexp/sexp.hpp"
#include "scheme.h"
#include <debug.hpp>
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
#if PLATFORM_WIN32

	auto stdHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	auto foreColor = FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED;
	SetConsoleTextAttribute(stdHandle, foreColor);
#endif
	auto scheme_env = setup_environment();
	for (;;){
		try{
			write(eval(read(), scheme_env));
		}
		catch (std::runtime_error& e){
			printf(e.what());
		}
	}
	return 0;
}


