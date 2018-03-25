/*! \file Core\Camera.h
\ingroup Engine
\brief 提供相机相关的数学计算。
*/

#ifndef LE_Core_Camera_H
#define LE_Core_Camera_H 1

#include <LBase/lmath.hpp>
#include <cmath>

namespace platform {
	namespace X {

		namespace lm = leo::math;
		inline lm::float4x4 perspective_fov_lh(float fov, float aspect, float nearPlane, float farPlane) noexcept {
			auto h = 1 / std::tanf(fov / 2);
			auto w = h / aspect;
			auto q = (farPlane / (farPlane - nearPlane));
			return {
				{ w,0,0,0 },
				{ 0,h,0,0 },
				{ 0,0,q,1 },
				{ 0,0,-nearPlane * q,0 }
			};
		}

		inline lm::float4x4 look_at_lh(const lm::float3& eye, const lm::float3& at, const lm::float3& up) noexcept {
			auto z = lm::normalize(at - eye);
			auto x = lm::normalize(lm::cross(up, z));
			auto y = lm::cross(z, x);

			return {
				{ x.x,y.x,z.x,0 },
				{ x.y,y.y,z.y,0 },
				{ x.z,y.z,z.z,0 },
				{ -lm::dot(x,eye),-lm::dot(y,eye),-lm::dot(z,eye),1 }
			};
		}
	}
}

#endif