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
		void GetProjectedShadowInitializer(const SceneInfo& scene,ProjectedShadowInitializer& initializer) const;

		Sphere GetShadowSplitBounds(const SceneInfo& scene) const;


	};
}