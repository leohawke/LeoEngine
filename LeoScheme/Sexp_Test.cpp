#include "Sexp/sexp.hpp"
#include "scheme.h"
#include <debug.hpp>
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
#if 0

	auto env = setup_environment();
	auto exp = input_parse("(define double (lambda (x) (+ 3.2 x)))");
	auto result = eval(exp, env);
	exp = input_parse("(double 3)");

	result = eval(exp,env);

	auto string = ops::print(result);

	printf("%s\n", string.c_str());
	//auto sexp = pack("tag", 1.0112345f, 2.f, 3.f);
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
