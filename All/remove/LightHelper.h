#pragma once

#include "leomathutility.hpp"
#include "Core\Material.h"
namespace leo
{
	struct DirectionLight
	{
		float4 ambient;
		float4 diffuse;
		float4 specular;
		float4 dir;//ignore w 
	};

	struct PointLight
	{
		float4 ambient;
		float4 diffuse;
		float4 specular;
		float4 position;//w : range
		float4 att;//ignore w;
	};

	struct SpotLight
	{
		float4 ambient;
		float4 diffuse;
		float4 specular;

		float4 position;//w : range

		float4 direction;//w : spot

		float4 att;//ignore w
	};
}