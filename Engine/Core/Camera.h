/*! \file Core\Camera.h
\ingroup Engine
\brief 提供相机相关的数学计算和基础数据类型。
\note 该文件实际上约束了左手坐标系的轴向描述
*/

#ifndef LE_Core_Camera_H
#define LE_Core_Camera_H 1

#include <LBase/lmath.hpp>
#include <cmath>

namespace LeoEngine {
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

namespace LeoEngine::Core {
	namespace lm = leo::math;

	class Camera {
	public:
		class TransformElement {
		public:
			TransformElement()
				:matrix(lm::float4x4::identity)
			{
			}

			lm::float3 GetEyePos() const noexcept {
				return  reinterpret_cast<const lm::float3&>(matrix[3].xyz);
			}
			void SetEyePos(lm::float3 pos) noexcept {
				matrix[3].xyz = pos;
			}

			lm::float3 GetRightVector() const noexcept {
				return  reinterpret_cast<const lm::float3&>(matrix[0].xyz);
			}
			void SetRightVector(lm::float3 right) noexcept {
				matrix[0].xyz = right;
			}
			lm::float3 GetUpVector() const noexcept {
				return  reinterpret_cast<const lm::float3&>(matrix[1].xyz);
			}
			void SetUpVector(lm::float3 up) noexcept {
				matrix[1].xyz = up;
			}

			lm::float3 GetForwardVector() const noexcept {
				return  reinterpret_cast<const lm::float3&>(matrix[2].xyz);
			}
			void SetForwardVector(lm::float3 forward) noexcept {
				matrix[2].xyz = forward;
			}

			lm::float4x4 GetViewMatrix() const noexcept {
				return lm::inverse(matrix);
			}

			void SetViewMatrix(const lm::float4x4& view) noexcept {
				matrix = lm::inverse(view);
			}
		private:
			lm::float4x4  matrix; // world-space matrix:translation view postiion to world
		};

		//Frustum Top Point in coordinate origin
		class FrustumElement {
		private:
			float fov;
			float aspect;
			float nearPlane;
			float farPlane;
		};
	public:
		lm::float3 GetEyePos() const noexcept {
			return transform_element.GetEyePos();
		}
		void SetEyePos(lm::float3 pos) noexcept {
			transform_element.SetEyePos(pos);
		}

		lm::float3 GetRightVector() const noexcept {
			return transform_element.GetRightVector();
		}
		void SetRightVector(lm::float3 right) noexcept {
			transform_element.SetRightVector(right);
		}
		lm::float3 GetUpVector() const noexcept {
			return  transform_element.GetUpVector();
		}
		void SetUpVector(lm::float3 up) noexcept {
			transform_element.SetUpVector(up);
		}

		lm::float3 GetForwardVector() const noexcept {
			return transform_element.GetForwardVector();
		}
		void SetForwardVector(lm::float3 forward) noexcept {
			transform_element.SetForwardVector(forward);
		}

		lm::float4x4 GetViewMatrix() const noexcept {
			return transform_element.GetViewMatrix();
		}

		void SetViewMatrix(const lm::float4x4& view) noexcept {
			transform_element.SetViewMatrix(view);
		}
	protected:
		TransformElement transform_element;
		FrustumElement frustum_elemnt;
	};
}


#endif