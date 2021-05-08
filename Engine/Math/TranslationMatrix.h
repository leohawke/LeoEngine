#pragma once
#include <LBase/lmath.hpp>


namespace LeoEngine
{
	namespace lm = leo::math;

	class TranslationMatrix :public lm::float4x4
	{
	public:
		explicit TranslationMatrix(lm::float3 delta)
			:
			lm::float4x4(
				lm::float4(1, 0, 0, 0),
				lm::float4(0, 1, 0, 0),
				lm::float4(0, 0, 1, 0),
				lm::float4(delta,   1)
			)
		{
		}
	};
}