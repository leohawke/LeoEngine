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

		float CascadeDistributionExponent = 3.0f;

		float FarShadowDistance = 3000.0f;
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

		float GetEffectiveCascadeDistributionExponent() const
		{
			return CascadeDistributionExponent;
		}
	public:
		static constexpr float ComputeAccumulatedScale(float Exponent, int32 CascadeIndex, int32 CascadeCount)
		{
			if (CascadeIndex <= 0)
			{
				return 0.0f;
			}

			float CurrentScale = 1;
			float TotalScale = 0.0f;
			float Ret = 0.0f;

			// lame implementation for simplicity, CascadeIndex is small anyway
			for (int i = 0; i < CascadeCount; ++i)
			{
				if (i < CascadeIndex)
				{
					Ret += CurrentScale;
				}
				TotalScale += CurrentScale;
				CurrentScale *= Exponent;
			}

			return Ret / TotalScale;
		}
	};
}