#pragma once
#include <LBase/lmath.hpp>


namespace LeoEngine
{
	namespace lm = leo::math;
	class SceneInfo
	{
	public:
		lm::float3 AABBMin;
		lm::float3 AABBMax;
	};
}