////////////////////////////////////////////////////////////////////////////
//
//  Leo Engine Source File.
//  Copyright (C), FNS Studios, 2014-2014.
// -------------------------------------------------------------------------
//  File name:   Core/camera.h
//  Version:     v1.01
//  Created:     ?/?/2014 by leo hawke.
//  Compilers:   Visual Studio.NET 2013
//  Description: 提供引擎所需的各种相机接口
// -------------------------------------------------------------------------
//  History:
//				12/4/2014 相机重写,虚函数引入
//
////////////////////////////////////////////////////////////////////////////

#ifndef Core_camera_h
#define Core_camera_h

#include "..\IndePlatform\Geometry.hpp"
namespace leo
{

	namespace default_param
	{
		const float frustum_near = 0.25f;
		const float frustum_far = 1024.0f;
		const float frustum_fov = 75.f / 180.f*LM_PI;
		const float frustum_aspect = 16.f / 9.f;
	}

	enum class PROJECTION_TYPE : std::uint8_t
	{
		ORTHOGRAPHIC,
		PERSPECTIVE
	};

	class LB_API CameraFrustum : public Frustum {
	public:
		CameraFrustum(const CameraFrustum& lvaue) = default;

		~CameraFrustum() = default;

		CameraFrustum(const float3& origin = float3(0.0f, 0.0f, 0.f), const float4& orientation = float4(0.0f, 0.0f, 0.0f, 1.0f), float fov = default_param::frustum_fov,
			float aspect = default_param::frustum_aspect, PROJECTION_TYPE projtype = PROJECTION_TYPE::PERSPECTIVE,
			float nearf = default_param::frustum_near, float farf = default_param::frustum_far)
			:Frustum(origin,orientation,nearf,farf),mProjType(projtype), mAspect(aspect), mFov(fov)
		{
			Update();
		}

		//don't update matrix
		CameraFrustum& SetFrustum(const float3& origin, const float4& orientation = float4(0.0f, 0.0f, 0.0f, 1.0f))
		{
			mOrientation = orientation;
			mOrigin = origin;
		}
		//don't update matrix
		CameraFrustum& SetFrustum(float fov, float aspect, float nearf, float farf)
		{
			mNear = nearf;
			mFar = farf;
			mFov = fov;
			mAspect = aspect;
		}
		//update matrix
		void SetFrustum(PROJECTION_TYPE projtype)
		{
			mProjType = projtype;
			Update();
		}

		float4x4 Proj() const {
			return mMatrix;
		}

		DefGetter(lnothrow, PROJECTION_TYPE&, Type, mProjType);

		DefGetter(lnothrow, float&, Fov, mFov);

		DefGetter(lnothrow, float&, Aspect, mAspect);

		DefGetter(lnothrow, float&, NearHeight, mNearHeight);
	private:
		void Update()
		{
			float sinfov, cosfov;
			sinfov = sincosr(&cosfov, mFov / 2.f);
			auto cotfov = static_cast<float>(cosfov / sinfov);
			auto tanfov = static_cast<float>(sinfov / cosfov);
			//for 正交投影计算
			mNearHeight = 2 * mNear / cotfov;
			auto width = mNearHeight*mAspect;
			float fRange = mFar / (mFar - mNear);
			switch (mProjType)
			{
			case leo::PROJECTION_TYPE::PERSPECTIVE:
				mMatrix(0, 0) = cotfov / mAspect;
				mMatrix(1, 1) = cotfov;
				break;
			case leo::PROJECTION_TYPE::ORTHOGRAPHIC:
				mMatrix(0, 0) = 2.0f / width;
				mMatrix(1, 1) = 2.0f / mNearHeight;
				break;
			default:
				break;
			}
			//矩阵的相同值
			{
				mMatrix(0, 1) = 0.0f;
				mMatrix(0, 2) = 0.0f;
				mMatrix(0, 3) = 0.0f;

				mMatrix(1, 2) = 0.0f;
				mMatrix(1, 3) = 0.0f;
				mMatrix(1, 0) = 0.0f;
				mMatrix(2, 0) = 0.0f;
				mMatrix(2, 1) = 0.0f;
				mMatrix(2, 2) = fRange;
				mMatrix(2, 3) = 1.0f;

				mMatrix(3, 0) = 0.0f;
				mMatrix(3, 1) = 0.0f;
				mMatrix(3, 2) = -fRange*mNear;
				mMatrix(3, 3) = 0.0f;
			}

			mRightSlope = +tanfov*mAspect;
			mLeftSlope = -tanfov*mAspect;
			mTopSlope = +tanfov;
			mBottomSlope = -tanfov;

			if (mProjType == PROJECTION_TYPE::ORTHOGRAPHIC)
			{
				mRightSlope /= mFar;
				mLeftSlope /= mFar;
				mTopSlope /= mFar;
				mBottomSlope /= mFar;
			}
		}
	protected:
		/// Orthographic(正交) or Perspective(投影)?
		PROJECTION_TYPE mProjType = PROJECTION_TYPE::PERSPECTIVE;

		/// y 视角域,默认 default_param::frustum_fov
		float mFov = default_param::frustum_fov;

		///横纵比,默认 default_param::frustum_aspect
		float mAspect = default_param::frustum_aspect;

		/// 近裁剪面高度
		float mNearHeight = 0;

		float4x4 mMatrix;
	};

	//基础相机类
	class LB_API Camera : private CameraFrustum, public GeneralAllocatedObject
	{
	private:
		float4x4 mMatrix;

		float3 mRight = float3(1.f,0.f,0.f);
		float3 mUp = float3(0.f,1.f,0.f);
		float3 mLook = float3(0.f, 0.f, 1.f);

		void Update()
		{
			//正交维持
			mLook = Normalize(mLook);
			mUp = Normalize(Cross(mLook, mRight));
			mRight = Cross(mUp, mLook);

			auto x = -Dot(mOrigin, mRight);
			auto y = -Dot(mOrigin, mUp);
			auto z = -Dot(mOrigin, mLook);

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

			mOrientation = MatrixToQuaternion(mMatrix);
		}
	protected:
		using CameraFrustum::mOrigin;
	public:
		Camera()
		:CameraFrustum(){
			Update();
		}
		~Camera() = default;
		using CameraFrustum::Proj;
		using CameraFrustum::GetType;
		using CameraFrustum::GetFov;
		using CameraFrustum::GetAspect;
		using CameraFrustum::GetNearHeight;

		//don't update proj matrix
		Camera& SetFrustum(float fov, float aspect, float nearf, float farf)
		{
			CameraFrustum::SetFrustum(fov, aspect, nearf, farf);
		}
		//update proj matrix
		void SetFrustum(PROJECTION_TYPE projtype)
		{
			CameraFrustum::SetFrustum(projtype);
		}

		float4x4 View() const
		{
			return mMatrix;
		}

		void UpdateViewMatrix() {
			Update();
		}

		const float3& GetRight()const//N
		{
			return mRight;
		}
		const float3& GetUp()const//V
		{
			return mUp;
		}
		const float3& GetLook()const//U
		{
			return mLook;
		}

		void LookAt(const float3& pos, const float3& target, const float3& up)
		{
			mOrigin = pos;
			mLook = Normalize(Subtract(target, pos));
			mRight = Normalize(Cross(up, mLook));
			mUp = Cross(mLook, mRight);
			Update();
		}

		virtual Camera& Walk(float d) = 0;
		virtual Camera& Strafe(float d) = 0;
		virtual Camera& Yaw(float angle) = 0;
		virtual Camera& Pitch(float angle) = 0;
		virtual Camera& Roll(float angle) = 0;
	};

	class LB_API UVNCamera : public Camera {
	public:
		UVNCamera() = default;

		void SetUVN(const float3& U_right, const float3& V_up, const float3& N_look)
		{
			mRight = U_right;
			mUp = V_up;
			mLook = N_look;
			UpdateViewMatrix();
		}

		//W/S
		Camera& Walk(float d)
		{
			mOrigin = MultiplyAdd(d, mLook, mOrigin);
			return *this;
		}
		//A/D
		Camera& Strafe(float d)
		{
			mOrigin = MultiplyAdd(d, mRight, mOrigin);
			return *this;
		}

		Camera& Yaw(float angle)
		{
			XMMATRIX R = XMMatrixRotationAxis(load(mUp), angle);
			save(mUp, XMVector3TransformNormal(load(mRight), R));
			save(mLook, XMVector3TransformNormal(load(mLook), R));
			return *this;
		}

		//RBUTTON UP/DOWN
		Camera& Pitch(float angle)
		{
			XMMATRIX R = XMMatrixRotationAxis(load(mRight), angle);
			save(mUp, XMVector3TransformNormal(load(mUp), R));
			save(mLook, XMVector3TransformNormal(load(mLook), R));
			return *this;
		}
		//MIDDBUTTON UP/DOWN
		Camera& Roll(float angle)
		{
			XMMATRIX R = XMMatrixRotationAxis(load(mLook), angle);
			save(mUp, XMVector3TransformNormal(load(mUp), R));
			save(mLook, XMVector3TransformNormal(load(mRight), R));
			return *this;
		}
		//RBUTTON LEFT/RIGHT
		Camera& RotateY(float angle)
		{
			// Rotate the basis vectors about the world y-axis.

			XMMATRIX R = XMMatrixRotationY(angle);

			save(mRight, XMVector3TransformNormal(load(mRight), R));
			save(mUp, XMVector3TransformNormal(load(mUp), R));
			save(mLook, XMVector3TransformNormal(load(mLook), R));
			return *this;
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
		float3 mRight;
		float3 mUp;
		float3 mLook;
	};

	//跟随相机
	//绑定SQTObject,并进行消息派发
	class FollowCamera : public Camera{
	public:
		FollowCamera(const std::reference_wrapper<SQTObject> followObject);
		~FollowCamera();
	public:
		//W/S
		void Walk(float d);
		//A/D
		void Strafe(float d);
		//Space

		void UpdateViewMatrix();

		void SetFrustum(float fov, float aspect, float nearf, float farf)
		{
			ViewFrustum::SetFrustum(fov, aspect, nearf, farf);
		}
		void SetFrustum(PROJECTION_TYPE projtype)
		{
			ViewFrustum::SetFrustum(projtype);
		}

		void Yaw(float angle);
		void Pitch(float angle);
		void Roll(float angle);
	};
}

#endif