#pragma once

#include <LBase/lmath.hpp>

namespace LeoEngine
{
	class ProjectedShadowInitializer
	{
	public:
		leo::math::float4x4 WorldToLight;
	};
}