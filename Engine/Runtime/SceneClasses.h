#pragma once

#include "Math/BoxSphereBounds.h"

namespace LeoEngine
{
	namespace lm = leo::math;

	class ProjectedShadowInitializer
	{
	public:
		lm::float4x4 WorldToLight;

		lm::float3 ShadowTranslation;

		lm::float3 Scales;

		lm::float4 WAxis;

		BoxSphereBounds SubjectBounds;
	};
}