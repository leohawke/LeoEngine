#pragma once

#include "..\IndePlatform\memory.hpp"
#include "..\leomath.hpp"
#include "Geometry.hpp"
namespace leo
{
	//UVN相机
	//Look方向 N
	//上方向 V
	// 右方向 U
	class Camera : public ViewFrustum, public GeneralAllocatedObject
	{
	public:
		Camera()
		{}
		~Camera() = default;
	private:
		//隐藏Set接口,不允许直接改变朝向
		using ViewFrustum::SetFrustum;
		void _update()
		{
			//正交维持
			mLook = Normalize(mLook);
			mUp = Normalize(Cross(mLook, mRight));
			mRight = Cross(mUp, mLook);

			auto x = -Dot(Origin, mRight);
			auto y = -Dot(Origin, mUp);
			auto z = -Dot(Origin, mLook);

			mMatrix(0, 0) = mRight.x;
			mMatrix(1, 0) = mRight.y;
			mMatrix(2, 0) = mRight.z;
			mMatrix(3, 0) = x;

			mMatrix(0, 1) = mUp.x;
			mMatrix(1, 1) = mUp.y;
			mMatrix(2, 1) = mUp.z;
			mMatrix(3, 1) = y;

			mMatrix(0, 2) = mLook.x;
			mMatrix(1, 2) = mLook.y;
			mMatrix(2, 2) = mLook.z;
			mMatrix(3, 2) = z;

			mMatrix(0, 3) = 0.0f;
			mMatrix(1, 3) = 0.0f;
			mMatrix(2, 3) = 0.0f;
			mMatrix(3, 3) = 1.0f;

			auto temp = MatrixToQuaternion(mMatrix);
			memcpy(Orientation, temp);
		}
	public:
		XMMATRIX View() const
		{
			return loadfloat4x4(&mMatrix);
		}
		using ViewFrustum::Proj;
		XMMATRIX ViewProj() const
		{
			return View()*Proj();
		}

		float3 GetRight()const//N
		{
			return mRight;
		}
		float3 GetUp()const//V
		{
			return mUp;
		}
		float3 GetLook()const//U
		{
			return mLook;
		}

		void SetFrustum(float fov, float aspect, float nearf, float farf)
		{
			ViewFrustum::SetFrustum(fov, aspect, nearf, farf);
		}
		void SetFrustum(PROJECTION_TYPE projtype)
		{
			ViewFrustum::SetFrustum(projtype);
		}

		void LookAt(const float3& pos, const float3& target, const float3& up)
		{
			memcpy(Origin, pos);
			mLook = Normalize(Subtract(target, pos));
			mRight = Normalize(Cross(up, mLook));
			mUp = Cross(mLook, mRight);
			_update();
		}

		void SetUVN(const float3& U_right, const float3& V_up, const float3& N_look)
		{
			mRight = U_right;
			mUp = V_up;
			mLook = N_look;
			_update();
		}

		//W/S
		void Walk(float d)
		{
			auto temp = MultiplyAdd(d, mLook, Origin);
			memcpy(Origin, temp);
		}
		//A/D
		void Strafe(float d)
		{
			auto temp = MultiplyAdd(d, mRight, Origin);
			memcpy(Origin, temp);
		}

		void Yaw(float angle)
		{
			XMMATRIX R = XMMatrixRotationAxis(load(mUp), angle);
			save(mUp, XMVector3TransformNormal(load(mRight), R));
			save(mLook, XMVector3TransformNormal(load(mLook), R));
		}
		//RBUTTON LEFT/RIGHT
		void RotateY(float angle)
		{
			// Rotate the basis vectors about the world y-axis.

			XMMATRIX R = XMMatrixRotationY(angle);

			save(mRight, XMVector3TransformNormal(load(mRight), R));
			save(mUp, XMVector3TransformNormal(load(mUp), R));
			save(mLook, XMVector3TransformNormal(load(mLook), R));
		}
		//RBUTTON UP/DOWN
		void Pitch(float angle)
		{
			XMMATRIX R = XMMatrixRotationAxis(load(mRight), angle);
			save(mUp, XMVector3TransformNormal(load(mUp), R));
			save(mLook, XMVector3TransformNormal(load(mLook), R));
		}
		//MIDDBUTTON UP/DOWN
		void Roll(float angle)
		{
			XMMATRIX R = XMMatrixRotationAxis(load(mLook), angle);
			save(mUp, XMVector3TransformNormal(load(mUp), R));
			save(mLook, XMVector3TransformNormal(load(mRight), R));
		}

		void UpdateViewMatrix()
		{
			_update();
		}

	public:
		inline ContainmentType Contains(FXMVECTOR V0, FXMVECTOR V1, FXMVECTOR V2) const
		{
			// Create 6 planes (do it inline to encourage use of registers)
			XMVECTOR NearPlane = XMVectorSet(0.0f, 0.0f, -1.0f, Near-0.1f);
			

			XMVECTOR FarPlane = XMVectorSet(0.0f, 0.0f, 1.0f, -Far-0.1f);
			

			XMVECTOR RightPlane = XMVectorSet(1.0f, 0.0f, -RightSlope-0.1f, 0.0f);
			

			XMVECTOR LeftPlane = XMVectorSet(-1.0f, 0.0f, LeftSlope-0.1f, 0.0f);
			

			XMVECTOR TopPlane = XMVectorSet(0.0f, 1.0f, -TopSlope-0.1f, 0.0f);
			

			XMVECTOR BottomPlane = XMVectorSet(0.0f, -1.0f, BottomSlope-0.1f, 0.0f);
			

			return TriangleTests::ContainedBy(V0, V1, V2, NearPlane, FarPlane, RightPlane, LeftPlane, TopPlane, BottomPlane);
		}

		inline ContainmentType Contains(FXMVECTOR V0) const
		{
			auto mView = View();

			XMVECTOR Planes[6];
			Planes[0] = XMVectorSet(0.0f, 0.0f, -1.0f, Near);
			Planes[1] = XMVectorSet(0.0f, 0.0f, 1.0f, -Far);
			Planes[2] = XMVectorSet(1.0f, 0.0f, -RightSlope, 0.0f);
			Planes[3] = XMVectorSet(-1.0f, 0.0f, LeftSlope, 0.0f);
			Planes[4] = XMVectorSet(0.0f, 1.0f, -TopSlope, 0.0f);
			Planes[5] = XMVectorSet(0.0f, -1.0f, BottomSlope, 0.0f);

			// Transform point into local space of frustum.
			XMVECTOR TPoint = V0;

			// Set w to one.
			TPoint = XMVectorInsert<0, 0, 0, 0, 1>(TPoint, XMVectorSplatOne());

			XMVECTOR Zero = XMVectorZero();
			XMVECTOR Outside = Zero;

			// Test point against each plane of the frustum.
			for (size_t i = 0; i < 6; ++i)
			{
				XMVECTOR Dot = XMVector4Dot(TPoint, Planes[i]);
				Outside = XMVectorOrInt(Outside, XMVectorGreater(Dot, Zero));
			}

			return XMVector4NotEqualInt(Outside, XMVectorTrueInt()) ? CONTAINS : DISJOINT;
		}
	private:
		float4x4 mMatrix;

		float3 mRight;
		float3 mUp;
		float3 mLook;
	};

	//跟随相机
	//绑定SQTObject,并进行消息派发
}