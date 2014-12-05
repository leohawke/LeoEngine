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
			:Frustum(origin, orientation, nearf, farf), mProjType(projtype), mAspect(aspect), mFov(fov)
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

		const float4x4& Proj() const {
			return mMatrix;
		}

		DefGetter(lnothrow const,const PROJECTION_TYPE&, Type, mProjType);

		DefGetter(lnothrow const, const float&, Fov, mFov);

		DefGetter(lnothrow const, const float&, Aspect, mAspect);

		DefGetter(lnothrow const, const float&, NearHeight, mNearHeight);

		DefGetter(lnothrow const, const float3&, Origin, mOrigin);
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
		void Update()
		{
			//正交维持
			auto vLook = Normalize<>(load(mLook));
			save(mLook,vLook);
			auto vRight = load(mRight);
			auto vUp = Normalize<>(Cross<>(vLook, vRight));
			save(mUp, vUp);
			vRight = Cross<>(vUp, vLook);
			save(mRight, vRight);

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

			save(ImplViewProj(), Multiply(load(mMatrix), load(Proj())));
		}

		float4x4& ImplViewProj() const{
			static float4x4 mViewProj;
			return mViewProj;
		}
	protected:
		using CameraFrustum::mOrigin;
		float4x4 mMatrix;

		float3 mRight = float3(1.f, 0.f, 0.f);
		float3 mUp = float3(0.f, 1.f, 0.f);
		float3 mLook = float3(0.f, 0.f, 1.f);
	public:
		Camera()
			:CameraFrustum() {
			Update();
		}
		~Camera() = default;
		using CameraFrustum::Proj;
		using CameraFrustum::GetType;
		using CameraFrustum::GetFov;
		using CameraFrustum::GetAspect;
		using CameraFrustum::GetNearHeight;
		using CameraFrustum::GetOrigin;
		using CameraFrustum::Intersects;
		using CameraFrustum::Contains;

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

		const float4x4& View() const
		{
			return mMatrix;
		}

		const float4x4& ViewProj() const {
			return ImplViewProj();
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

		Camera& Rotation(const float3& Axis, float angle)
		{
			// Rotate the basis vectors about the world y-axis.

			auto R = RotationAxis(load(Axis), angle);

			save(mRight, TransformNormal<>(load(mRight), R));
			save(mUp, TransformNormal<>(load(mUp), R));
			save(mLook, TransformNormal<>(load(mLook), R));
			return *this;
		}
		Camera& Rotation(const float4& quaternion) {
			auto R = Matrix(load(quaternion));
			save(mRight, TransformNormal<>(load(mRight), R));
			save(mUp, TransformNormal<>(load(mUp), R));
			save(mLook, TransformNormal<>(load(mLook), R));
			return *this;
		}
		//\params matrix3x3
		Camera& Rotation(const float4x4& matrix) {
			auto R = load(matrix);
			save(mRight, TransformNormal<>(load(mRight), R));
			save(mUp, TransformNormal<>(load(mUp), R));
			save(mLook, TransformNormal<>(load(mLook), R));
			return *this;
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


		Camera& Walk(float d)
		{
			mOrigin = MultiplyAdd(d, mLook, mOrigin);
			return *this;
		}

		Camera& Strafe(float d)
		{
			mOrigin = MultiplyAdd(d, mRight, mOrigin);
			return *this;
		}

		Camera& Yaw(float angle)
		{
			auto R = RotationAxis(load(mUp), angle);
			save(mUp, TransformNormal<>(load(mRight), R));
			save(mLook, TransformNormal<>(load(mLook), R));
			return *this;
		}

		Camera& Pitch(float angle)
		{
			auto R = RotationAxis(load(mRight), angle);
			save(mUp, TransformNormal<>(load(mUp), R));
			save(mLook, TransformNormal<>(load(mLook), R));
			return *this;
		}

		Camera& Roll(float angle)
		{
			auto R = RotationAxis(load(mLook), angle);
			save(mUp, TransformNormal<>(load(mUp), R));
			save(mLook, TransformNormal<>(load(mRight), R));
			return *this;
		}
	};

	//跟随相机
	//绑定SQTObject,并进行消息派发
	class LB_API FollowCamera : public Camera {
	public:
		struct camera_dir {};

		explicit FollowCamera(const std::reference_wrapper<SQTObject>& followObject)
			:mFollowObject(followObject) {
		}
		~FollowCamera() = default;

		Camera& ChangeObject(const std::reference_wrapper<SQTObject>& followObject) {
			mFollowObject = followObject;
		}

		Camera& Walk(float d) {
			auto offset = float3(0.f, 0.f, d);
			mFollowObject.get().Translation(offset);
			mOrigin = Add(offset, mOrigin);
			return *this;
		}
		
		Camera& Strafe(float d) {
			auto offset = float3(d, 0.f, 0.f);
			mFollowObject.get().Translation(offset);
			mOrigin = Add(offset, mOrigin);
			return *this;
		}
		
		Camera& Yaw(float angle) {
			auto q = EulerAngleToQuaternion(float3(0.f, 0.f,angle));
			mFollowObject.get().Rotation(q);
			Rotation(q);
		}
		Camera& Pitch(float angle) {
			auto q = EulerAngleToQuaternion(float3(0.f, angle,0.f));
			mFollowObject.get().Rotation(q);
			Rotation(q);
		}
		Camera& Roll(float angle) {
			auto q = EulerAngleToQuaternion(float3(angle, 0.f, 0.f));
			mFollowObject.get().Rotation(q);
			Rotation(q);
		}
		

		//前后行走
		//\param camera_dir :摄像机mLook方向
		Camera& Walk(float d, camera_dir) {
			float3 offset;
			save(offset, Multiply(load(mLook), Splat(d)));
			mFollowObject.get().Translation(offset);
			mOrigin = Add(offset, mOrigin);
			return *this;
		}
		//左右行走
		//\param camera_dir :摄像机mRight方向
		Camera& Strafe(float d, camera_dir)
		{
			float3 offset;
			save(offset, Multiply(load(mRight), Splat(d)));
			mFollowObject.get().Translation(offset);
			mOrigin = Add(offset, mOrigin);
			return *this;
		}

		//\param camera_dir :摄像机mUp方向
		Camera& Yaw(float angle, camera_dir) {
			mFollowObject.get().Rotation(mUp, angle);
			Rotation(mUp, angle);
		}
		//\param camera_dir :摄像机mRight方向
		Camera& Pitch(float angle, camera_dir) {
			mFollowObject.get().Rotation(mRight, angle);
			Rotation(mRight, angle);
		}
		//\param camera_dir :摄像机mLook方向
		Camera& Roll(float angle, camera_dir) {
			mFollowObject.get().Rotation(mLook, angle);
			Rotation(mLook, angle);
		}
	private:
		std::reference_wrapper<SQTObject> mFollowObject;
	};
}

#endif