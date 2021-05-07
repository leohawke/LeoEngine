#include "DirectionalLight.h"
#include "Math/InverseRotationMatrix.h"
using namespace LeoEngine;

void DirectionalLight::GetProjectedShadowInitializer(ProjectedShadowInitializer& initializer) const
{
	initializer.WorldToLight = InverseRotationMatrix(Rotator(lm::normalize(GetDirection())));
}