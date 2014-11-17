#include "Skeleton.hpp"
#include "..\IndePlatform\utility.hpp"

namespace leo{
	SkeletonInstance::SkeletonInstance(const std::shared_ptr<SkeletonData>& skeData)
		:mSkeData(skeData),mNorT(0.f) {
		mAniIndex = mSkeData->mAnimations.begin()->first;
		for (auto & a : mSkeData->mAnimaNames)
			mSpeedPerAni[a] = 1.f;
		mSkinMatrix = leo::make_unique<float4x4[]>(mSkeData->mSkeleton.mJointCount);
	}
	SkeletonInstance::~SkeletonInstance(){

	}


	bool SkeletonInstance::SwitchAnimation(const std::wstring& aniName){
		return SwitchAnimation(aniName.c_str());
	}
	bool SkeletonInstance::SwitchAnimation(const wchar_t* aniName){
		auto Index = hash(aniName);
		if (std::find(mSkeData->mAnimaNames.begin(), mSkeData->mAnimaNames.end(), Index) == mSkeData->mAnimaNames.end())
			return false;
		mAniIndex = Index;
		return true;
	}

	//do many thing
	void SkeletonInstance::Update(){

	}
	void SkeletonInstance::Render(ID3D11DeviceContext* context, const Camera& camera){

	}
}