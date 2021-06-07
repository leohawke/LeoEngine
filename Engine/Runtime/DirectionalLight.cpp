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
	auto NumTotalCascades = GetNumViewDependentWholeSceneShadows(scene);

	const uint32 ShadowSplitIndex = CascadeIndex;

	float SplitNear = GetSplitDistance(scene, ShadowSplitIndex);
	float SplitFar = GetSplitDistance(scene, ShadowSplitIndex+1);
	float FadePlane = SplitFar;

	float LocalCascadeTransitionFraction = CascadeTransitionFraction * GetShadowTransitionScale();

	float FadeExtension = (SplitFar - SplitNear) * LocalCascadeTransitionFraction;

	// Add the fade region to the end of the subfrustum, if this is not the last cascade.
	if ((int32)ShadowSplitIndex < (int32)NumTotalCascades - 1)
	{
		SplitFar += FadeExtension;
	}

	if (OutCascadeSettings)
	{
		OutCascadeSettings->SplitNear = SplitNear;
		OutCascadeSettings->SplitFar = SplitFar;
		OutCascadeSettings->ShadowSplitIndex = (int32)ShadowSplitIndex;
		OutCascadeSettings->FadePlaneOffset = FadePlane;
		OutCascadeSettings->FadePlaneLength = SplitFar - FadePlane;
	}

	const float BoundsCalcNear = SplitNear;

	const Sphere CascadeSphere = GetShadowSplitBoundsDepthRange(scene, scene.Matrices.GetViewOrigin(), BoundsCalcNear, SplitFar, OutCascadeSettings);

	return CascadeSphere;
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

Sphere DirectionalLight::GetShadowSplitBoundsDepthRange(const SceneInfo& scene, lm::float3 ViewOrigin, float SplitNear, float SplitFar, ShadowCascadeSettings* OutCascadeSettings) const
{
	auto ViewMatrix = scene.Matrices.GetViewMatrix();
	auto ProjMatrix = scene.Matrices.GetProjectionMatrix();

	const auto CameraDirection = ViewMatrix.GetColumn(2);

	float Aspect = ProjMatrix[1][1] / ProjMatrix[0][0];

	float HalfVerticalFOV = std::atan(1.0f / ProjMatrix[1][1]);
	float HalfHorizontalFOV = std::atan(1.0f / ProjMatrix[0][0]);

	float AsymmetricFOVScaleX = ProjMatrix[2][0];
	float AsymmetricFOVScaleY = ProjMatrix[2][1];

	// Near plane
	const float StartHorizontalTotalLength = SplitNear * std::tan(HalfHorizontalFOV);
	const float StartVerticalTotalLength = SplitNear * std::tan(HalfVerticalFOV);
	const lm::float3 StartCameraLeftOffset = ViewMatrix.GetColumn(0) * -StartHorizontalTotalLength * (1 + AsymmetricFOVScaleX);
	const lm::float3 StartCameraRightOffset = ViewMatrix.GetColumn(0) * StartHorizontalTotalLength * (1 - AsymmetricFOVScaleX);
	const lm::float3 StartCameraBottomOffset = ViewMatrix.GetColumn(1) * -StartVerticalTotalLength * (1 + AsymmetricFOVScaleY);
	const lm::float3 StartCameraTopOffset = ViewMatrix.GetColumn(1) * StartVerticalTotalLength * (1 - AsymmetricFOVScaleY);
	// Far plane
	const float EndHorizontalTotalLength = SplitFar * std::tan(HalfHorizontalFOV);
	const float EndVerticalTotalLength = SplitFar * std::tan(HalfVerticalFOV);
	const lm::float3 EndCameraLeftOffset = ViewMatrix.GetColumn(0) * -EndHorizontalTotalLength * (1 + AsymmetricFOVScaleX);
	const lm::float3 EndCameraRightOffset = ViewMatrix.GetColumn(0) * EndHorizontalTotalLength * (1 - AsymmetricFOVScaleX);
	const lm::float3 EndCameraBottomOffset = ViewMatrix.GetColumn(1) * -EndVerticalTotalLength * (1 + AsymmetricFOVScaleY);
	const lm::float3 EndCameraTopOffset = ViewMatrix.GetColumn(1) * EndVerticalTotalLength * (1 - AsymmetricFOVScaleY);

	// Get the 8 corners of the cascade's camera frustum, in world space
	lm::float3 CascadeFrustumVerts[8];
	CascadeFrustumVerts[0] = ViewOrigin + CameraDirection * SplitNear + StartCameraRightOffset + StartCameraTopOffset;    // 0 Near Top    Right
	CascadeFrustumVerts[1] = ViewOrigin + CameraDirection * SplitNear + StartCameraRightOffset + StartCameraBottomOffset; // 1 Near Bottom Right
	CascadeFrustumVerts[2] = ViewOrigin + CameraDirection * SplitNear + StartCameraLeftOffset + StartCameraTopOffset;     // 2 Near Top    Left
	CascadeFrustumVerts[3] = ViewOrigin + CameraDirection * SplitNear + StartCameraLeftOffset + StartCameraBottomOffset;  // 3 Near Bottom Left
	CascadeFrustumVerts[4] = ViewOrigin + CameraDirection * SplitFar + EndCameraRightOffset + EndCameraTopOffset;       // 4 Far  Top    Right
	CascadeFrustumVerts[5] = ViewOrigin + CameraDirection * SplitFar + EndCameraRightOffset + EndCameraBottomOffset;    // 5 Far  Bottom Right
	CascadeFrustumVerts[6] = ViewOrigin + CameraDirection * SplitFar + EndCameraLeftOffset + EndCameraTopOffset;       // 6 Far  Top    Left
	CascadeFrustumVerts[7] = ViewOrigin + CameraDirection * SplitFar + EndCameraLeftOffset + EndCameraBottomOffset;    // 7 Far  Bottom Left

	// Calculate the ideal bounding sphere for the subfrustum.
	//https://lxjk.github.io/2017/04/15/Calculate-Minimal-Bounding-Sphere-of-Frustum.html

	float k = std::sqrt(1 + 1 /(Aspect * Aspect)) * std::tan(HalfHorizontalFOV);

	Sphere CascadeSphere(ViewOrigin + CameraDirection * SplitFar, SplitFar*k);

	float k2 = k * k;
	if (k2 < (SplitFar - SplitNear) / (SplitFar + SplitNear))
	{
		float k4 = k2 * k2;

		float CentreZ = 0.5f * (SplitFar + SplitNear) * (1 + k2);

		CascadeSphere.Center = ViewOrigin + CameraDirection * CentreZ;

		CascadeSphere.W = 0.5f * std::sqrt(
			(SplitFar - SplitNear)* (SplitFar - SplitNear)+
			2*(SplitFar* SplitFar+ SplitNear* SplitNear)*k2+
			(SplitFar+SplitNear)* (SplitFar + SplitNear)*k4
		);
	}

	return CascadeSphere;
}

