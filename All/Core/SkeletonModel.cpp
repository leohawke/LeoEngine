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

			std::vector<MeshFile::Joint> joints(ske_header.numjoint);

			std::vector<MeshFile::JointAnimaSample> anima(ske_header.numframe);
			for (auto & data : anima){
				data.data = make_unique<float[]>(ske_header.numframe * 16);
			}

			mSkeleton = leo::make_shared<Skeleton>();
			mSkeleton->mJointCount = ske_header.numjoint;


			mSkeleton->mJoints = std::make_unique<Joint[]>(mSkeleton->mJointCount);

			for (auto i = 0u; i != mSkeleton->mJointCount; ++i)
				mSkeleton->mJoints[i] =convert(joints[i]);

			AnimationClip mClip;
			mClip.mSkeleton = mSkeleton;
			mClip.mFCount =ske_header.loop? ske_header.numframe :ske_header.numframe-1;
			mClip.mSamples = std::make_unique<AnimationSample[]>(mClip.mFCount);
			for (auto i = 0u; i != mClip.mFCount; ++i){
				mClip.mSamples[i].mJointsPose = std::make_unique<JointPose[]>(ske_header.numjoint);
			}

			for (auto f = 0u; f != mClip.mFCount; ++f)
				for (auto j = 0u; j != ske_header.numjoint; ++j){
				mClip.mSamples[f].mJointsPose[j] = &anima[j].data[f * 16];
				}
			mAnimation.SetData(std::move(mClip));
		}
		Catch_Win32_Exception
	}
}

#pragma warning( pop )

