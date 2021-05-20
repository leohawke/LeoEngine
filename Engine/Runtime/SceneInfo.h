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

		lm::float4x4 ViewMatrix;
		lm::float4x4 ProjectionMatrix;

		lm::float3 ViewOrigin;

		float NearClippingDistance;

		int32 MaxShadowCascades = 10;
	};
}