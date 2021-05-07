#pragma once
#include <LBase/lmath.hpp>


namespace LeoEngine
{
	namespace lm = leo::math;

	struct Sphere
	{
		lm::float3 Center;

		float W;

		Sphere()
			:Center(0,0,0)
			,W(0)
		{

		}

		friend bool operator==(const Sphere& l, const Sphere& r)
		{
			return (l.Center == r.Center) && lm::float_equal(l.W, r.W);
		}
	};
}