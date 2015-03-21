#include "leo_math_convert_impl.h"
#include "clock.hpp"

#include <cstdio>

int main()
{
	auto f = leo::details::half_to_float(0X0800);
	auto h = leo::details::float_to_half(f);
	printf("%f\n", f);
	printf("%u\n", h);
	auto clock = leo::Clock();
	clock.Update();
	for (size_t i = 0; i != 10000; ++i)
	{
		f = leo::details::half_to_float(h);
		f += 0.0000001f;
		h = leo::details::float_to_half(f);
	}
	clock.Update();
	printf("%lu\n", clock.GetDelta().count());

	printf("%f\n", f);
	printf("%u\n", h);
	return 0;
}