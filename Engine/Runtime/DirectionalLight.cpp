#include "DirectionalLight.h"
#include "Math/InverseRotationMatrix.h"
using namespace LeoEngine;

void DirectionalLight::GetProjectedShadowInitializer(const SceneInfo& scene, ProjectedShadowInitializer& initializer) const
{
	auto Bounds = GetShadowSplitBounds(scene);

	initializer.ShadowTranslation = -Bounds.Center;
	initializer.WorldToLight = InverseRotationMatrix(Rotator(lm::normalize(GetDirection())));
	initializer.Scales = lm::float3(1 / Bounds.W, 1 / Bounds.W,1);
	initializer.WAxis = lm::float4(0, 0, 0, 1);

	auto ShadowExtent = Bounds.W / sqrt(3.0f);
	initializer.SubjectBounds = BoxSphereBounds(lm::float3(), lm::float3(ShadowExtent, ShadowExtent, ShadowExtent), Bounds.W);
}

Sphere DirectionalLight::GetShadowSplitBounds(const SceneInfo& scene) const
{
	auto Center = (scene.AABBMin + scene.AABBMax) / 2.0f;
	auto Radius = lm::length(scene.AABBMax - scene.AABBMin) / 2;

	Sphere CascadeSpere(Center, Radius);

	return CascadeSpere;
}