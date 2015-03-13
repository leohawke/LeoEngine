////////////////////////////////////////////////////////////////////////////
//
//  Leo Engine Source File.
//  Copyright (C), FNS Studios, 2014-2014.
// -------------------------------------------------------------------------
//  File name:   Core/Skeleton.h
//  Version:     v1.00
//  Created:     10/27/2014 by leo hawke.
//  Compilers:   Visual Studio.NET 2013
//  Description: 提供骨骼层相关逻辑
// -------------------------------------------------------------------------
//  History:
//				
//
////////////////////////////////////////////////////////////////////////////

#ifndef Core_Skeleton_H
#define Core_Skeleton_H

#include "..\IndePlatform\LeoMath.h"
#include "..\IndePlatform\utility.hpp"
#include "..\Core\CoreObject.hpp"
namespace leo{
	struct Joint : public GeneralAllocatedObject{
		//绑定姿势的逆变换
		float4x4 mInvBindPose;
		//字符串散列标识符
		std::size_t mNameSid;
		//父节点的下标,侵入式设计->Skeleton存放所有Joint
		std::uint8_t mParent;

		const wchar_t* GetName() const{
			return unhash(mNameSid);
		}

		void SetName(const wchar_t* str){
			mNameSid = hash(str);
		}
	};

	struct Skeleton{
		//关节数组
		std::unique_ptr<Joint[]> mJoints;
		//关节数目
		std::uint8_t mJointCount;
	};

	//关节姿势,不允许非统一缩放
	struct JointPose : public SQTObject{
		JointPose& operator=(const SQT& sqt){
			SQT::operator=(sqt);
			return *this;
		}

		JointPose(JointPose&& rhs)
			:SQTObject(rhs){
		}

		JointPose() {
		}

		//This function onlu uses the rotation portion of the JointPose
		//See ..\LeoMath.hpp FILE 225 LINE
		JointPose operator*(const JointPose& pose){
			//assert(s == 1.f);
			JointPose result;
			float4x4 matrix;
			save(matrix, Multiply(this->operator std::array<__m128, 4U>(), pose.operator std::array<__m128, 4U>()));
			save(result.q,Quaternion(load(matrix)));
			result.s = 1.f;
			result.t = float3(0.f,0.f, 0.f);
			return result;
		}

		JointPose& operator=(JointPose&& rvalue){
			SQT::operator=(rvalue);
		}
	};

	struct SkeletonPose{
		//骨骼,存放关节数目
		std::shared_ptr<Skeleton> mSkeleton;
		//多个局部关节姿势
		std::unique_ptr<JointPose[]> mLocalPoses = nullptr;
		//多个全局关节姿势,矩阵相乘之后无法存放至SQT对象

		std::unique_ptr<float4x4Object[]> mGlobalPoses = nullptr;

		std::unique_ptr<float4x4Object[]> mSkinMatrixs = nullptr;

		SkeletonPose& operator=(SkeletonPose&& rvalue){
			mSkeleton = std::move(rvalue.mSkeleton);
			mLocalPoses = std::move(rvalue.mLocalPoses);
			mGlobalPoses = std::move(rvalue.mGlobalPoses);
			mSkinMatrixs = std::move(rvalue.mSkinMatrixs);
			return *this;
		}

		SkeletonPose(SkeletonPose&& rvalue)
			:mSkeleton(std::move(rvalue.mSkeleton)),
			mLocalPoses(std::move(rvalue.mLocalPoses)),
		mGlobalPoses(std::move(rvalue.mGlobalPoses)),
		mSkinMatrixs(std::move(rvalue.mSkinMatrixs)) {

		}

		SkeletonPose() = default;

	};

}

#endif