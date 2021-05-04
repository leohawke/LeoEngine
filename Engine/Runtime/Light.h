#pragma once

#include <LBase/lmath.hpp>


namespace LeoEngine
{
	namespace lm = leo::math;

	class Light
	{
	private:
		/** A transform from light space into world space. */
		lm::float4x4 LightToWorld;
		/** A transform from world space into light space. */
		lm::float4x4 WorldToLight;
	public:
		void SetTransform(const lm::float4x4& light2world);

		lm::float3 GetDirection() const;
	};
}