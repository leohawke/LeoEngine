////////////////////////////////////////////////////////////////////////////
//
//  Leo Engine Source File.
//  Copyright (C), FNS Studios, 2014-2014.
// -------------------------------------------------------------------------
//  File name:   Core/Joint.hpp
//  Version:     v1.00
//  Created:     10/28/2014 by leo hawke.
//  Compilers:   Visual Studio.NET 2013
//  Description: 提供骨骼层动画相关逻辑
// -------------------------------------------------------------------------
//  History:
//				
//
////////////////////////////////////////////////////////////////////////////

#ifndef Core_SkeletonAnimation_Hpp
#define Core_SkeletonAnimation_Hpp

#include "Skeleton.hpp"

namespace leo{
	struct AnimationSample{
		//一个被侵入的类,大小取决于关节数目->AnimationClip.mSkeleton.mJointCount;
		std::unique_ptr<JointPose[]> mJointsPose;
	};

	struct AnimationClip{
		//骨骼,存放关节数目
		std::unique_ptr<Skeleton> mSkeleton;
		//每秒多少帧
		float mFPS;
		//帧数目
		std::uint8_t mFCount;
		std::unique_ptr<AnimationSample[]> mSamples;
		bool mLoop;
		//if ture => arrsize(mSamples) = mFCount;
		//else => arrsiez(mSamples) => mFCount +1;
	};

	class Animation{
		AnimationClip mClip;
		float t;
	};

	//在这里计算蒙皮调色板,返回为一堆矩阵的引用,即SkeltonPose的引用
	//需要实现插值函数,两个时间,辅助函数<static 单列>
}
#endif