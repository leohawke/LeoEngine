#ifndef IndePlatform_LeoMathEx_hpp
#define IndePlatform_LeoMathEx_hpp


#include "leomathutility.hpp"

namespace leo {
	float3 TransformNormal(const float3& normal, const float4x4& matrix) {
		float3 result;
		save(result, TransformNormal(load(normal), load(matrix)));
		return result;
	}
}


#endif
