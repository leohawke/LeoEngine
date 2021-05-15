#include "DirectionalLight.h"
#include "Math/InverseRotationMatrix.h"
using namespace LeoEngine;

void DirectionalLight::GetProjectedShadowInitializer(const SceneInfo& scene, int32 CascadeIndex, WholeSceneProjectedShadowInitializer& initializer) const
{
	auto Bounds = GetShadowSplitBounds(scene,CascadeIndex,&initializer.CascadeSettings);
	initializer.CascadeSettings.ShadowSplitIndex = CascadeIndex;

	initializer.ShadowTranslation = -Bounds.Center;
	initializer.WorldToLight = InverseRotationMatrix(Rotator(lm::normalize(GetDirection())));
	initializer.Scales = lm::float3(1 / Bounds.W, 1 / Bounds.W,1);
	initializer.WAxis = lm::float4(0, 0, 0, 1);

	auto ShadowExtent = Bounds.W / sqrt(3.0f);
	initializer.SubjectBounds = BoxSphereBounds(lm::float3(), lm::float3(ShadowExtent, ShadowExtent, ShadowExtent), Bounds.W);
}

constexpr int32 MaxNumFarShadowCascades = 10;

uint32 DirectionalLight::GetNumViewDependentWholeSceneShadows(const SceneInfo& scene) const
{
	auto ClampedFarShadowCascadeCount = std::min<uint32>(MaxNumFarShadowCascades, FarShadowCascadeCount);

	auto TotalCascades = GetNumShadowMappedCascades(scene.MaxShadowCascades) + ClampedFarShadowCascadeCount;

	return TotalCascades;
}

Sphere DirectionalLight::GetShadowSplitBounds(const SceneInfo& scene, int32 CascadeIndex, ShadowCascadeSettings* OutCascadeSettings) const
{
	auto NumTotalCascade = GetNumViewDependentWholeSceneShadows(scene);

	const uint32 ShadowSplitIndex = CascadeIndex;

	float SplitNear = GetSplitDistance(scene, ShadowSplitIndex);
	float SplitFar = GetSplitDistance(scene, ShadowSplitIndex+1);

	auto Center = (scene.AABBMin + scene.AABBMax) / 2.0f;
	auto Radius = lm::length(scene.AABBMax - scene.AABBMin) / 2;

	Sphere CascadeSpere(Center, Radius);

	return CascadeSpere;
}

uint32 DirectionalLight::GetNumShadowMappedCascades(uint32 MaxShadowCascades) const
{
	int32 EffectiveNumDynamicShadowCascades = DynamicShadowCascades;

	const int32 NumCascades = GetCSMMaxDistance(MaxShadowCascades) > 0.0f ? EffectiveNumDynamicShadowCascades : 0;

	return std::min<uint32>(NumCascades,MaxShadowCascades);
}

float DirectionalLight::GetCSMMaxDistance(int32 MaxShadowCascades) const
{
	auto DistanceSclae = 1.0f;

	if (MaxShadowCascades <= 0)
	{
		return 0.0f;
	}

	float Scale = lm::clamp(0.0f, 2.0f, DistanceSclae);

	float Distance = GetEffectiveWholeSceneDynamicShadowRadius() * Scale;

	return Distance;
}


float DirectionalLight::GetSplitDistance(const SceneInfo& scene, uint32 SplitIndex) const
{
	auto NumCascades = GetNumShadowMappedCascades(scene.MaxShadowCascades);

	float CascadeDistance = GetCSMMaxDistance(scene.MaxShadowCascades);

	float ShadowNear = scene.NearClippingDistance;

	const float CascadeDistribution = GetEffectiveCascadeDistributionExponent();

	if (SplitIndex > NumCascades)
	{
		auto ClampedFarShadowCascadeCount = std::min<uint32>(MaxNumFarShadowCascades, FarShadowCascadeCount);

		return CascadeDistance + ComputeAccumulatedScale(CascadeDistribution, SplitIndex- NumCascades, ClampedFarShadowCascadeCount) * (FarShadowDistance - CascadeDistance);
	}
	else
	{
		return ShadowNear + ComputeAccumulatedScale(CascadeDistribution, SplitIndex, NumCascades) * (CascadeDistance - ShadowNear);
	}

	return 0.0f;
}
