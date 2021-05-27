#pragma once

#include "Math/BoxSphereBounds.h"
#include <LBase/linttype.hpp>

namespace LeoEngine
{
	using namespace leo::inttype;

	constexpr float SCENE_MAX = 2097152.0;

	class ShadowCascadeSettings
	{
	public:
		float SplitNear = 0;

		float SplitFar = SCENE_MAX;

		int32 ShadowSplitIndex = -1;

		float ShadowCascadeBiasDistribution = 1.0f;

		float FadePlaneOffset = SplitFar;

		float FadePlaneLength = SplitFar-FadePlaneOffset;
	};

	class ProjectedShadowInitializer
	{
	public:
		lm::float4x4 WorldToLight;

		lm::float3 ShadowTranslation;

		lm::float3 Scales;

		lm::float4 WAxis;

		BoxSphereBounds SubjectBounds;
	};

	class WholeSceneProjectedShadowInitializer :public ProjectedShadowInitializer
	{
	public:
		ShadowCascadeSettings CascadeSettings;
	};
}