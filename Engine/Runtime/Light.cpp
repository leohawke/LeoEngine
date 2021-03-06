#include "Light.h"

using namespace LeoEngine;

void Light::SetTransform(const lm::float4x4& light2world)
{
	LightToWorld = light2world;

	WorldToLight = lm::inverse(LightToWorld);
}

lm::float3 Light::GetDirection() const
{
	return lm::float3(
		WorldToLight[0][2],
		WorldToLight[1][2],
		WorldToLight[2][2]);
}