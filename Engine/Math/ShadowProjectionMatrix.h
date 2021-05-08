#pragma once
#include <LBase/lmath.hpp>


namespace LeoEngine
{
	namespace lm = leo::math;

	class ShadowProjectionMatrix :public lm::float4x4
	{
	public:
		explicit ShadowProjectionMatrix(float MinZ, float MaxZ,lm::float4 WAxis)
			:lm::float4x4(
				lm::float4(1, 0, 0, WAxis.x),
				lm::float4(0, 1, 0, WAxis.y),
				lm::float4(0, 0, (WAxis.z* MaxZ + WAxis.w) / (MaxZ - MinZ), WAxis.z),
				lm::float4(0, 0, -MinZ * (WAxis.z * MaxZ + WAxis.w) / (MaxZ - MinZ), WAxis.w)
			)
		{}
	};
}