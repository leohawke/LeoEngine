#pragma once
#include <LBase/lmath.hpp>
#include <LBase/linttype.hpp>


namespace LeoEngine
{
	namespace lm = leo::math;
	using namespace leo::inttype;

	class SceneInfo
	{
	public:
		lm::float3 AABBMin;
		lm::float3 AABBMax;

		int32 MaxShadowCascades = 10;
	};
}