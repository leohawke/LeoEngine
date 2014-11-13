#ifndef LEO_SCHEME_Math_H
#define LEO_SCHEME_Math_H
#include "scheme.h"
namespace leo{
	namespace scheme{
		scheme_value scheme_and(const scheme_list& args);
		scheme_value scheme_or(const scheme_list& args);
		scheme_value scheme_not(const scheme_list& args);

		scheme_value scheme_abs(const scheme_list& args);
	}
}
#endif