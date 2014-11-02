#include "..\IndePlatform\platform.h"
#include "..\d3dx11.hpp"
#include "Mesh.hpp"

#include "SkeletonModel.hpp"

#include "MeshLoad.hpp"
#include "..\file.hpp"

#pragma warning( push )
#pragma warning(disable:4316)
namespace leo{

	static Joint convert(const MeshFile::Joint& joint){
		Joint result;
		result.mInvBindPose = float4x4(joint.data);
		result.mNameSid = hash(joint.name);
		result.mParent = joint.parent;
		return result;
	}

	struct SkeletonVertexAnimationInfo{
		uint32 mIndices;
		float3 mWeights;
		SkeletonVertexAnimationInfo(const MeshFile::SkeletonAdjInfo& info)
			:mIndices(info.indices),mWeights(info.weights){
		}
		SkeletonVertexAnimationInfo& operator=(const MeshFile::SkeletonAdjInfo& info){
			mIndices = info.indices;
			mWeights = info.weights;
		}
	};

	//dst 采样,关节
	//src 关节,采样
	static void convertAnimaSample(std::unique_ptr<AnimationSample[]>& dst, std::unique_ptr < MeshFile::JointAnimaSample[]>& src, std::uint32_t numFrame, std::uint32_t numJoint){
		for (auto f = 0u; f != numFrame; ++f)
			for (auto j = 0u; j != numJoint; ++j){
				dst[f].mJointsPose[j] = src[j].data[f];
			}
	}

	void SkeletonModel::LoadFromFile(const std::wstring& filename, ID3D11Device* device){
		mMesh.Load(filename, device);

		if (filename.size() == 0)
			return;
		auto fin = win::File::OpenNoThrow(filename, win::File::TO_READ);
		try{
			leo::MeshFile::MeshFileHeader l3d_header;
			std::uint64_t fileoffset = 0;
			fin->Read(&l3d_header, sizeof(l3d_header), fileoffset);
			fileoffset += sizeof(l3d_header);

			fileoffset += sizeof(MeshFile::MeshMaterial)*l3d_header.numsubset;
			fileoffset += sizeof(MeshFile::MeshSubSet)*l3d_header.numsubset;
			fileoffset += sizeof(MeshFile::MeshVertex)*l3d_header.numvertice;
			fileoffset += sizeof(std::uint32_t) * l3d_header.numindex;

			leo::MeshFile::SkeletonHeader ske_header;
			fin->Read(&ske_header, sizeof(ske_header), fileoffset);
			fileoffset += sizeof(ske_header);


			std::vector<leo::MeshFile::SkeletonAdjInfo> adjinfos(l3d_header.numvertice);
			fin->Read(&adjinfos[0], sizeof(MeshFile::SkeletonAdjInfo)*adjinfos.size(), fileoffset);

			fileoffset += sizeof(MeshFile::SkeletonAdjInfo)*adjinfos.size();
			try{
				std::vector<SkeletonVertexAnimationInfo> animationdatas(adjinfos.begin(), adjinfos.end());
				CD3D11_BUFFER_DESC aniVbdesc; (sizeof(SkeletonVertexAnimationInfo)*animationdatas.size(), D3D11_BIND_VERTEX_BUFFER, D3D11_USAGE_IMMUTABLE);
				D3D11_SUBRESOURCE_DATA aniSubDesc;
				aniSubDesc.pSysMem = animationdatas.data();
				dxcall(device->CreateBuffer(&aniVbdesc, &aniSubDesc, &mAnimationDataBUffer));
			}
			Catch_DX_Exception


			std::vector<MeshFile::Joint> joints(ske_header.numjoint);
			fin->Read(&adjinfos[0], sizeof(MeshFile::Joint)*joints.size(), fileoffset);
			fileoffset += sizeof(MeshFile::Joint)*joints.size();
		


			std::vector<MeshFile::AnimatClip> anima(ske_header.numanima);
			for (auto & data : anima)
				data.data = make_unique<MeshFile::JointAnimaSample[]>(ske_header.numjoint);
			
			//Todo:
			mSkeleton = leo::make_shared<Skeleton>();
			mSkeleton->mJointCount = ske_header.numjoint;


			mSkeleton->mJoints = std::make_unique<Joint[]>(mSkeleton->mJointCount);

			for (auto i = 0u; i != mSkeleton->mJointCount; ++i)
				mSkeleton->mJoints[i] =convert(joints[i]);

			AnimationClip mClip;
			
			wchar_t name[260];
			std::uint32_t numframe;
			for (auto & clip : anima){
				//读入名字和帧数量
				fin->Read(name, sizeof(name), fileoffset);
				fileoffset += sizeof(name);
				fin->Read(&numframe, sizeof(uint32), fileoffset);
				fileoffset += sizeof(uint32);

				//分配时间内存并读入
				clip.timedata = std::make_unique<float[]>(numframe);
				fin->Read(clip.timedata.get(), sizeof(float)*numframe, fileoffset);
				fileoffset += sizeof(float)*numframe;

				//写入部分信息
				mClip.mSkeleton = mSkeleton;
				mClip.mFCount = numframe-1;
				mClip.mLoop = true;

				//为读入分配足够内存
				for (auto j = 0u; j != ske_header.numjoint; ++j)
					clip.data[j].data = std::make_unique<SeqSQT[]>(numframe);

				//开始读入
				for (auto j = 0u; j != ske_header.numjoint; ++j){
					fin->Read(clip.data[j].data.get(), sizeof(SeqSQT)*numframe, fileoffset);
					fileoffset += sizeof(SeqSQT)*numframe;
				}

				

				//转换读入SampleInfo至AnimationSample
				//1.分配内存
				//2.写入时间信息
				mClip.mSamples = std::make_unique<AnimationSample[]>(numframe);
				for (auto f = 0u; f != numframe; ++f){
					mClip.mSamples[f].mJointsPose = std::make_unique<JointPose[]>(mSkeleton->mJointCount);
					mClip.mSamples[f].mTimePoint = clip.timedata[f];
				}

				convertAnimaSample(mClip.mSamples, clip.data, numframe, mSkeleton->mJointCount);

				auto sid = hash(name);
				mAnimations.emplace(sid, Animation());
				mAnimations[sid].SetData(std::move(mClip));
			}
		

		}
		Catch_Win32_Exception
	}
}

#pragma warning( pop )

