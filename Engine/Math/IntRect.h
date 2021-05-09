#pragma once
#include <LBase/lmath.hpp>
#include <LBase/linttype.hpp>

namespace LeoEngine
{
	namespace lm = leo::math;

	using namespace leo::inttype;

	using IntPoint = lm::int2;

	struct IntRect
	{
		IntPoint Min;

		IntPoint Max;

		IntRect()
			:Min(),Max()
		{}

		IntRect(IntPoint min,IntPoint max)
			:Min(min),Max(max)
		{}
	};
}