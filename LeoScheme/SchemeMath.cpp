#include "SchemeMath.h"

namespace leo{
	namespace scheme{
		scheme_value scheme_and(const scheme_list& args){
			if (length(args) < 1)
				error("and至少有一个参数");
			auto  first = args->mCar;
			auto rest = args->mCdr;
			auto fuck = ops::print(rest);
			for (;;){
				if (!true_exp(first))
					return scheme_atom(false);
				if (!null(rest)){
					first = car(rest);
					rest = cdr(rest);
				}
				else
					break;
			}
			return first;
		}
		scheme_value scheme_or(const scheme_list& args){
			if (length(args) < 1)
				error("and至少有一个参数");
			auto first = args->mCar;
			auto rest = args->mCdr;
			for (;;){
				if (true_exp(first))
					return first;
				if (!null(rest)){
					first = car(rest);
					rest = cdr(rest);
				}
				else
					break;
			}
			return scheme_atom(false);
		}
		scheme_value scheme_not(const scheme_list& args){
			if (length(args) != 1)
				error("not只能有一个参数");
			return !true_exp(args->mCar);
		}

		scheme_value scheme_abs(const scheme_list& args){
			if (length(args) != 1 || !number(args->mCar))
				error("abs只能有一个参数,参数只能为数字");
			auto & arg0 = args->mCar;
			if (arg0.cast_atom().can_cast<scheme_int>()){
				auto i = arg0.cast_atom().cast<scheme_int>();
				if (i < 0)
					return scheme_atom(-i);
				else
					return scheme_atom(i);
			}
			else{
				auto f = arg0.cast_atom().cast<scheme_real>();
				if (f < 0.f)
					return scheme_atom(-f);
				else
					return scheme_atom(f);
			}

		}
	}
}