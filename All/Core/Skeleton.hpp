////////////////////////////////////////////////////////////////////////////
//
//  Leo Engine Source File.
//  Copyright (C), FNS Studios, 2014-2014.
// -------------------------------------------------------------------------
//  File name:   Core/Skeleton.h
//  Version:     v1.00
//  Created:     10/27/2014 by leo hawke.
//  Compilers:   Visual Studio.NET 2013
//  Description: 提供骨骼层相关接口
// -------------------------------------------------------------------------
//  History:
//				11/05/2014 接口,逻辑层移入底层
//
////////////////////////////////////////////////////////////////////////////
#ifndef Core_Skeleton_Hpp
#define Core_Skeleton_Hpp


#include "..\IndePlatform\LeoMath.h"
#include "..\IndePlatform\ldef.h"
#include "..\IndePlatform\memory.hpp"
#include "..\IndePlatform\clock.hpp"
#include "Lod.h"
#include "Material.h"
#include "Vertex.hpp"
#include "CoreObject.hpp"
#include <vector>
#include<map>

struct ID3D11Buffer;
struct ID3D11ShaderResourceView;
namespace leo{
	//骨骼数据(顶点信息,材质信息,动画信息)
	struct SkeletonData;

	//骨骼实例(骨骼数据可被多个实例共享)
	class SkeletonInstance;

	//未实现
	class SkeletonGroup;

	class Camera;


	struct AnimationSample;
	struct AnimationClip;

	struct SkeletonData{
		~SkeletonData();

		//能使用的最大LOD索引,最低的细节
		const static std::uint8_t MinLodLevel = 3;

		//从文件载入<参数:文件名>
		static std::shared_ptr<SkeletonData> Load(const std::wstring& fileName);

		static std::shared_ptr<SkeletonData> Load(const wchar_t* fileName){
			return SkeletonData::Load(std::wstring(fileName));
		}
		//从内存载入,未实现
		static std::shared_ptr<SkeletonData> Load(const MemoryChunk& memory);

		//Mesh begin
		ID3D11Buffer* mVertexBuffer = nullptr;
		//IndicesBuffer,Not IndiceBuffers
		ID3D11Buffer* mIndicesBuffer = nullptr;
		struct SubSet{
			LodIndex mLodIndices[MinLodLevel + 1];
			Material mMat;
			ID3D11ShaderResourceView* mTexSRV = nullptr;
			ID3D11ShaderResourceView* mNormalSRV = nullptr;
		};
		std::vector<SubSet> mSubSets;
		using vertex = Vertex::NormalMap;
		using vertex_adj = Vertex::SkeAdjInfo;
		//Mesh end

		//Skeleton begin
		struct Joint : public GeneralAllocatedObject{
			//绑定姿势的逆变换
			float4x4 mInvBindPose;
			//字符串散列标识符
			std::size_t mNameSid = 0;
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
			std::unique_ptr<Joint[]> mJoints;
			//关节数目
			std::uint8_t mJointCount;
		} mSkeleton;

		using JointPose = SQTObject;

		ID3D11Buffer* mAnimationDataBUffer = nullptr;
		//Skeleton end

		std::map<std::size_t, AnimationClip> mAnimations;
		std::vector<std::size_t> mAnimaNames;
	};

	struct AnimationSample{
		std::unique_ptr<SkeletonData::JointPose[]> mJointsPose;
		float mTimePoint;
	};

	struct AnimationClip{
		std::uint8_t mFCount = 0;
		std::unique_ptr<AnimationSample[]> mSamples;
		bool mLoop = true;

		AnimationClip() = default;

		AnimationClip(AnimationClip&& rvalue)
			:mFCount(rvalue.mFCount), mSamples(std::move(rvalue.mSamples)), mLoop(rvalue.mLoop){

		}

		//单位,秒
		float GetTotalTime() const;

		//单位,帧
		float CalcFrame(float t) const;
	};

	class SkeletonInstance : public SQTObject{
		//共享的骨骼数据
		std::shared_ptr<SkeletonData> mSkeData;
		//动画索引(名字hash后的值)
		std::size_t  mAniIndex;
		//标准时间点(range[0.f,1.f])
		float mNorT;
		float mElapsed = clock::GameClock::Now<>();
		//每个动画的播放速率
		std::map<std::size_t, float> mSpeedPerAni;
		//蒙皮矩阵
		std::unique_ptr<float4x4Object[]> mSkinMatrixs;
		std::unique_ptr<SkeletonData::JointPose[]> mLocalPoses;
		std::unique_ptr<float4x4Object[]> mGlobalPoses;
	public:
		SkeletonInstance(const std::shared_ptr<SkeletonData>& skeData);
		~SkeletonInstance();

		SkeletonInstance() = default;
		SkeletonInstance& operator=(const std::shared_ptr<SkeletonData>& skeData);

		bool SwitchAnimation(const wchar_t* aniName);

		void SetCurrentAniSpeed(float speed){
			mSpeedPerAni[mAniIndex] = speed;
 		}

		std::vector<const wchar_t*> GetAniNames() const;

		void Update();
		void Render(const Camera& camera);
	};

	
}

#endif