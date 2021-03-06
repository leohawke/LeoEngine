#pragma once

#ifndef LE_RENDER_DATASTRUCTURES_H
#define LE_RENDER_DATASTRUCTURES_H 1

#include <LBase/linttype.hpp>
#include <LBase/lmath.hpp>

namespace LeoEngine::Render {
	using namespace leo::inttype;
	namespace lm = leo::math;

	using LightIndex = uint16;

	enum DirectLightType {
		POINT_LIGHT = 0,
		SPOT_LIGHT =1,
		DIRECTIONAL_LIGHT =2
	};

	struct DirectLight {
		//direction for spot and directional lights (world space).
		lm::float3 direction;
		//range for point and spot lights(Maximum distance of influence)
		float range;
		//position for spot and point lights(world Space)
		lm::float3 position;
		//outer angle for spot light(radian)
		float outerangle;
		//blub size for point light or inneragnle for spot light
		float blub_innerangle;

		//The color of emitted light(linear RGB color)
		lm::float3 color;
		//
		uint32 type;
	};
}

#endif