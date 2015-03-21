#include "..\IndePlatform\platform.h"
#include "..\IndePlatform\utility.hpp"


#include "EffectSkeleton.hpp"

#include "Camera.hpp"

#include "..\d3dx11.hpp"
#include "..\DeviceMgr.h"
#include "..\ShaderMgr.h"

#include "Skeleton.hpp"


namespace leo{
	float AnimationClip::GetTotalTime() const {
		return mSamples[mFCount].mTimePoint;
	}

	float AnimationClip::CalcFrame(float t) const {
		if (t == 1.f)
			if (mLoop)
				return mFCount + 0.5f;
			else
				return float(mFCount);
		auto time = t*GetTotalTime();
		auto f = 0u;
		for (; f != mFCount - 1; ++f)
			if ((time >= mSamples[f].mTimePoint) && (time < mSamples[f + 1].mTimePoint))
				break;
		auto delta = mSamples[f + 1].mTimePoint - mSamples[f].mTimePoint;
		auto frac = (time - mSamples[f].mTimePoint) / delta;
		return f + frac;
	}

	SkeletonInstance::SkeletonInstance(const std::shared_ptr<SkeletonData>& skeData)
		:mSkeData(skeData),mNorT(0.f),mPlayAni(false) {
		mAniIndex = mSkeData->mAnimations.begin()->first;
		for (auto & a : mSkeData->mAnimaNames)
			mSpeedPerAni[a] = 1.f;
		mSkinMatrixs = leo::make_unique<float4x4Object[]>(mSkeData->mSkeleton.mJointCount);

		mLocalPoses = leo::make_unique<SkeletonData::JointPose[]>(mSkeData->mSkeleton.mJointCount);
		mGlobalPoses = leo::make_unique<float4x4Object[]>(mSkeData->mSkeleton.mJointCount);
		ReCurrAniBindPose();
	}
	SkeletonInstance::~SkeletonInstance(){
	}


	SkeletonInstance& SkeletonInstance::operator=(const std::shared_ptr<SkeletonData>& skeData){
		mPlayAni = false;
		mSkeData = skeData;
		mNorT = (0.f);
		mAniIndex = mSkeData->mAnimations.begin()->first;
		for (auto & a : mSkeData->mAnimaNames)
			mSpeedPerAni[a] = 1.f;
		mSkinMatrixs = leo::make_unique<float4x4Object[]>(mSkeData->mSkeleton.mJointCount);

		mLocalPoses = leo::make_unique<SkeletonData::JointPose[]>(mSkeData->mSkeleton.mJointCount);
		mGlobalPoses = leo::make_unique<float4x4Object[]>(mSkeData->mSkeleton.mJointCount);
		ReCurrAniBindPose();
		return *this;
	}

	
	bool SkeletonInstance::SwitchAnimation(const wchar_t* aniName){
		auto Index = hash(aniName);
		if (std::find(mSkeData->mAnimaNames.begin(), mSkeData->mAnimaNames.end(), Index) == mSkeData->mAnimaNames.end())
			return false;
		mAniIndex = Index;
		return true;
	}

	std::vector<const wchar_t*> SkeletonInstance::GetAniNames() const {
		std::vector<const wchar_t*> aniNames(mSkeData->mAnimaNames.size());
		std::transform(mSkeData->mAnimaNames.cbegin(), mSkeData->mAnimaNames.cend(), aniNames.begin(), [](std::size_t sid){return unhash(sid); });
		return aniNames;
	}


	void SkeletonInstance::BeginCurrentAni() {
		mPlayAni = true;
	}
	void SkeletonInstance::EndCurrentAni() {
		mPlayAni = false;
		ReCurrAniBindPose();
	}

	void SkeletonInstance::ReCurrAniBindPose() {
		auto & mClip = mSkeData->mAnimations[mAniIndex];
		for (auto jointIndex = 0u; jointIndex != mSkeData->mSkeleton.mJointCount; ++jointIndex) {
			mLocalPoses[jointIndex] = mClip.mSamples[0].mJointsPose[jointIndex];
		}

		mGlobalPoses[0] = mLocalPoses[0].operator leo::float4x4();
		//子节点在后面,只需找到父,即可相乘
		for (auto jointIndex = 1u; jointIndex != mSkeData->mSkeleton.mJointCount; ++jointIndex) {
			auto & joint = mSkeData->mSkeleton.mJoints[jointIndex];
			auto parentToRoot = load(static_cast<const float4x4&>(mGlobalPoses[joint.mParent]));
			auto toParent = mLocalPoses[jointIndex].operator std::array<__m128, 4U>();
			save(mGlobalPoses[jointIndex], Multiply(toParent, parentToRoot));
		}
		for (auto jointIndex = 0u; jointIndex != mSkeData->mSkeleton.mJointCount; ++jointIndex) {
			auto invBindPose = load(mSkeData->mSkeleton.mJoints[jointIndex].mInvBindPose);
			auto toRoot = load(static_cast<const float4x4&>(mGlobalPoses[jointIndex]));
			save(mSkinMatrixs[jointIndex], Multiply(invBindPose, toRoot));
		}

		mNorT = 0.f;
	}

	//do many thing
	void SkeletonInstance::Update(){
		if (!mPlayAni)
			return;
		auto & mClip = mSkeData->mAnimations[mAniIndex];
		if (mNorT == 1.f)
			if (mClip.mLoop)
				mNorT = 0.f;
			else
				return ;
		auto cElapsed = clock::GameClock::Now<>();
		auto ElapsedT = (cElapsed - mElapsed) / mClip.GetTotalTime();
		mElapsed = cElapsed;

		mNorT += (ElapsedT*mSpeedPerAni[mAniIndex]);
		clamp(0.f, 1.f, mNorT);

		auto  CalcFrameIndex = [&](float frame){
			auto first = (uint32)(std::floor(frame));
			auto second = first + 1u;

			if (mClip.mLoop)
				first %= (mClip.mFCount + 1), second %= (mClip.mFCount + 1);
			else
				clamp(0u, unsigned int(mClip.mFCount), second);
			return std::pair<uint32,uint32>{ first, second };
		};
		auto CalcFrameInterpolate = [](float frame){
			return frame - std::floor(frame);
		};


		auto frame = mClip.CalcFrame(mNorT);
		auto Indices = CalcFrameIndex(frame);


		for (auto jointIndex = 0u; jointIndex != mSkeData->mSkeleton.mJointCount; ++jointIndex){
			auto & p1 = mClip.mSamples[Indices.first].mJointsPose[jointIndex];
			auto & p2 = mClip.mSamples[Indices.second].mJointsPose[jointIndex];

			mLocalPoses[jointIndex] = Lerp(p1, p2, CalcFrameInterpolate(frame));
		}

		mGlobalPoses[0] = mLocalPoses[0].operator leo::float4x4();
		//子节点在后面,只需找到父,即可相乘
		for (auto jointIndex = 1u; jointIndex != mSkeData->mSkeleton.mJointCount; ++jointIndex){
			auto & joint = mSkeData->mSkeleton.mJoints[jointIndex];
			auto parentToRoot = load(static_cast<const float4x4&>(mGlobalPoses[joint.mParent]));
			auto toParent = mLocalPoses[jointIndex].operator std::array<__m128, 4U>();
			save(mGlobalPoses[jointIndex], Multiply(toParent, parentToRoot));
		}
		for (auto jointIndex = 0u; jointIndex != mSkeData->mSkeleton.mJointCount; ++jointIndex){
			auto invBindPose = load(mSkeData->mSkeleton.mJoints[jointIndex].mInvBindPose);
			auto toRoot = load(static_cast<const float4x4&>(mGlobalPoses[jointIndex]));
			save(mSkinMatrixs[jointIndex], Multiply(invBindPose, toRoot));
		}
	}
	void SkeletonInstance::Render(const Camera& camera){
		auto context = DeviceMgr().GetDeviceContext();
		context->IASetIndexBuffer(mSkeData->mIndicesBuffer, DXGI_FORMAT_R32_UINT, 0);
		static UINT strides[] = { sizeof(SkeletonData::vertex), sizeof(SkeletonData::vertex_adj) };
		static UINT offsets[] = { 0, 0 };
		ID3D11Buffer* vbs[] = {mSkeData->mVertexBuffer,mSkeData->mAnimationDataBUffer };
		context->IASetVertexBuffers(0, 2, vbs, strides, offsets);
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		context->IASetInputLayout(ShaderMgr().CreateInputLayout(InputLayoutDesc::Skinned));

		auto & mEffect = EffectSkeleton::GetInstance();

		auto world =SQT::operator std::array<__m128, 4U>();
		auto viewproj = load(camera.ViewProj());
		mEffect->WorldMatrix(world);
		mEffect->WorldViewProjMatrix(Multiply(world,viewproj));
		mEffect->SkinMatrix(mSkinMatrixs.get(),mSkeData->mSkeleton.mJointCount);
		mEffect->Apply(context);

		for (auto it = mSkeData->mSubSets.cbegin(); it != mSkeData->mSubSets.cend(); ++it)
		{
			mEffect->Mat(it->mMat, context);
			mEffect->DiffuseSRV(it->mTexSRV,context);
			mEffect->NormalMapSRV(it->mNormalSRV, context);
			context->DrawIndexed(it->mLodIndices[0].mCount, it->mLodIndices[0].mOffset, 0);
		}

		if (EffectConfig::GetInstance()->NormalLine())
		{
			auto & mLineEffect = EffectNormalLine::GetInstance();
			context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
			mLineEffect->World(world);
			mLineEffect->ViewProj(camera.ViewProj());
			mLineEffect->Apply(context);
			for (auto it = mSkeData->mSubSets.cbegin(); it != mSkeData->mSubSets.cend(); ++it)
			{
				context->DrawIndexed(it->mLodIndices[0].mCount, it->mLodIndices[0].mOffset, 0);
			}
		}
	}

	void SkeletonInstance::CastShadow() {
		//Todo :
		//通过VS生成变换后的顶点,然后绘制
	}
}