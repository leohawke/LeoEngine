#pragma once

#include "Light.h"
#include "SceneClasses.h"
#include "Math/Sphere.h"
#include "SceneInfo.h"

namespace LeoEngine
{
	namespace lm = leo::math;

	class DirectionalLight : public Light
	{
	public:
		int32 FarShadowCascadeCount = 0;

		int32 DynamicShadowCascades = 3;

		float WholeSceneDynamicShadowRadius = 200;
	public:
		void GetProjectedShadowInitializer(const SceneInfo& scene,int32 CascadeIndex, WholeSceneProjectedShadowInitializer& initializer) const;

		uint32 GetNumViewDependentWholeSceneShadows(const SceneInfo& scene) const;

		Sphere GetShadowSplitBounds(const SceneInfo& scene, int32 CascadeIndex,ShadowCascadeSettings* OutCascadeSettings) const;

	private:
		uint32 GetNumShadowMappedCascades(uint32 MaxShadowCascades) const;

		float GetCSMMaxDistance(int32 MaxShadowCascades) const;

		float GetEffectiveWholeSceneDynamicShadowRadius() const
		{
			return WholeSceneDynamicShadowRadius;
		}

		float GetSplitDistance(const SceneInfo& scene, uint32 SplitIndex) const;
	};
}