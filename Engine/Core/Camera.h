/*! \file Core\Camera.h
\ingroup Engine
\brief 提供相机基础数据类型。
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

		inline lm::float4 CreateInvDeviceZToWorldZTransform(const lm::float4x4& ProjMatrix)
		{
			// The perspective depth projection comes from the the following projection matrix:
			//
			// | 1  0  0  0 |
			// | 0  1  0  0 |
			// | 0  0  A  1 |
			// | 0  0  B  0 |
			//
			// Z' = (Z * A + B) / Z
			// Z' = A + B / Z
			//
			// So to get Z from Z' is just:
			// Z = B / (Z' - A)
			//
			// Note a reversed Z projection matrix will have A=0.
			//
			// Done in shader as:
			// Z = 1 / (Z' * C1 - C2)   --- Where C1 = 1/B, C2 = A/B
			//

			float DepthMul = ProjMatrix[2][2];
			float DepthAdd = ProjMatrix[3][2];

			if (DepthAdd == 0.f)
			{
				// Avoid dividing by 0 in this case
				DepthAdd = 0.00000001f;
			}

			// perspective
			// SceneDepth = 1.0f / (DeviceZ / ProjMatrix[3][2] - ProjMatrix[2][2] / ProjMatrix[3][2])

			// ortho
			// SceneDepth = DeviceZ / ProjMatrix[2][2] - ProjMatrix[3][2] / ProjMatrix[2][2];

			bool bIsPerspectiveProjection = ProjMatrix[3][3] < 1.0f;

			if (bIsPerspectiveProjection)
			{
				float SubtractValue = DepthMul / DepthAdd;

				// Subtract a tiny number to avoid divide by 0 errors in the shader when a very far distance is decided from the depth buffer.
				// This fixes fog not being applied to the black background in the editor.
				SubtractValue -= 0.00000001f;

				return lm::float4(
					0.0f,
					0.0f,
					1.0f / DepthAdd,
					SubtractValue
				);
			}
			else
			{
				return lm::float4(
					1.0f / ProjMatrix[2][2],
					-ProjMatrix[3][2] / ProjMatrix[2][2] + 1.0f,
					0.0f,
					1.0f
				);
			}
		}
	}
}

namespace LeoEngine::Core {
	namespace lm = leo::math;

	//TODO!
	//我们应该使用艺术工作者所使用工具的坐标系[3D-MAX]
	//! "right-handed" coordinate systems, where the positive X-Axis points
	//! to the right, the positive Y-Axis points away from the viewer and the positive
	//! Z-Axis points up. The following illustration shows our coordinate system.
	//! <PRE>
	//! z-axis
	//!  ^
	//!  |
	//!  |   y-axis
	//!  |  /
	//!  | /
	//!  |/
	//!  +---------------->   x-axis
	//! </PRE>

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
			float fov = 75.f/180*3.14f;
			float aspect = 1;
			float nearPlane = 0.25f;
			float farPlane = 1024.f;
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

		void SetFrustum(leo::uint16 width, leo::uint16 height, const FrustumElement& frustum = {});

		leo::uint16 GetFrustumViewWidth() const { return width; }
		leo::uint16 GetFrustumViewHeight() const { return height; }
	protected:
		TransformElement transform_element;
		FrustumElement frustum_elemnt;

		leo::uint16 width;
		leo::uint16 height;
	};
}


#endif