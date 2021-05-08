#pragma once
#include <LBase/lmath.hpp>


namespace LeoEngine
{
	namespace lm = leo::math;

	class ScaleMatrix :public lm::float4x4
	{
	public:
		explicit ScaleMatrix(lm::float3 scale)
			:lm::float4x4(
				lm::float4(scale.x,  0,     0,      0),
				lm::float4(0,     scale.y,  0,      0),
				lm::float4(0,        0,    scale.z, 0),
				lm::float4(0,        0,      0,     1)
			)
		{}
	};
}