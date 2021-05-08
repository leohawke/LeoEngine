#pragma once
#include <LBase/lmath.hpp>


namespace LeoEngine
{
	namespace lm = leo::math;

	struct BoxSphereBounds
	{
		BoxSphereBounds()
			:Origin(),
			Extent(),
			Radius(0)
		{
		}

		lm::float3 Origin;
		
		lm::float3 Extent;

		float Radius;

		BoxSphereBounds(lm::float3 origin,lm::float3 extent, float r)
			:Origin(origin),
			Extent(extent),
			Radius(r)
		{
		}
	};
}