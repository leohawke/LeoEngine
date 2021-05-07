#pragma once

#include "Light.h"
#include "SceneClasses.h"

namespace LeoEngine
{
	namespace lm = leo::math;

	class DirectionalLight : public Light
	{
	public:
		void GetProjectedShadowInitializer(ProjectedShadowInitializer& initializer) const;
	};
}