#include "..\IndePlatform\platform.h"
#include "..\d3dx11.hpp"

#include "Skeleton.hpp"

#include "..\TextureMgr.h"
#include "..\ShaderMgr.h"
#include "..\RenderStates.hpp"
#include "..\file.hpp"
#include "..\exception.hpp"
#include "..\DeviceMgr.h"
#include "MeshLoad.hpp"
namespace leo{

	SkeletonData::~SkeletonData(){
		leo::win::ReleaseCOM(mVertexBuffer);
		leo::win::ReleaseCOM(mIndicesBuffer);
		leo::win::ReleaseCOM(mAnimationDataBUffer);
		for (auto & s : mSubSets){
			leo::win::ReleaseCOM(s.mNormalSRV);
			leo::win::ReleaseCOM(s.mTexSRV);
		}
	}

	static void ReadSubSets(const MemoryChunk& min, std::uint64_t& offset, std::vector<SkeletonData::SubSet>&subsets, ID3D11Device* device)
	{
		auto size = subsets.size();
		std::vector<MeshFile::MeshMaterial> materials(size);
		min.Read(&materials[0], sizeof(MeshFile::MeshMaterial)*size, offset);
		offset += sizeof(MeshFile::MeshMaterial)*size;
		std::vector<MeshFile::MeshSubSet> filesubsets(size);
		min.Read(&filesubsets[0], sizeof(MeshFile::MeshSubSet)*size, offset);
		offset += sizeof(MeshFile::MeshSubSet)*size;
		leo::TextureMgr texmgr;
		for (UINT i = 0; i < size; ++i)
		{
			subsets[i].mTexSRV = texmgr.LoadTextureSRV(materials[i].diffusefile);
			subsets[i].mNormalSRV = texmgr.LoadTextureSRV(materials[i].normalmapfile);
			subsets[i].mLodIndices[0].mCount = filesubsets[i].indexcount;
			subsets[i].mLodIndices[0].mOffset = filesubsets[i].indexoffset;
			BZero(subsets[i].mMat);
			std::memcpy(&subsets[i].mMat.ambient, &materials[i].ambient, sizeof(XMFLOAT3));
			std::memcpy(&subsets[i].mMat.diffuse, &materials[i].diffuse, sizeof(XMFLOAT3));
			std::memcpy(&subsets[i].mMat.specular, &materials[i].specular, sizeof(XMFLOAT3));
			subsets[i].mMat.specular.w = materials[i].specPow;
			//subsets[i].m_mat.reflect = materials[i].reflect;
		}
	}

	inline  static void ReadVertices(const MemoryChunk& min, std::uint64_t& offset, std::vector<SkeletonData::vertex>& vertices)
	{
		auto size = vertices.size();
		std::vector<MeshFile::MeshVertex> _vertices(size);
		auto buffersize = _vertices.size() * sizeof(MeshFile::MeshVertex);
		min.Read(&_vertices[0], buffersize, offset);

		for (std::size_t i = 0; i != size; ++i)
		{
			vertices[i] = _vertices[i];
		}

		offset += buffersize;
	}

	inline  static void ReadIndices(const MemoryChunk& min, std::uint64_t& offset, std::vector<std::uint32_t>& indices)
	{
		auto size = indices.size() * sizeof(std::uint32_t);
		min.Read(&indices[0], size, offset);
		offset += size;
	}

	inline static void ReadLodIndices(const MemoryChunk& min, std::uint64_t& offset, std::vector<SkeletonData::SubSet>&subsets, std::vector<std::uint32_t>& indices){
		std::uint32_t lodCount = 0;
		min.Read(&lodCount, 4, offset);
		offset += 4;
		assert(lodCount < 5 && lodCount > 1);
		auto lodIndicesCount = leo::make_unique<std::uint32_t[]>(lodCount - 1);
		min.Read(lodIndicesCount.get(), 4 * (lodCount - 1), offset);
		offset += (4 * (lodCount - 1));

		auto IndicesCount = [&]{
			auto c = 0u;
			for (auto i = 0u; i != lodCount - 1; ++i)
				c += lodIndicesCount[lodCount];
			return c;
		}();
		auto baseIndex = indices.size();
		indices.resize(indices.size() + IndicesCount);
		min.Read(&indices[baseIndex], IndicesCount * 4, offset);
		offset += IndicesCount * 4;

		auto size = subsets.size();
		std::vector<MeshFile::MeshSubSet> filesubsets((lodCount - 1)*size);
		min.Read(&filesubsets[0], sizeof(MeshFile::MeshSubSet)*filesubsets.size(), offset);
		for (auto i = 1u; i != lodCount; ++i){
			for (auto s = 0u; s != size; ++s){
				subsets[s].mLodIndices[i].mCount = filesubsets[(i - 1)*size + s].indexcount;
				subsets[s].mLodIndices[i].mOffset = filesubsets[(i - 1)*size + s].indexoffset;
			}
		}
		offset += (sizeof(MeshFile::MeshSubSet)*filesubsets.size());
	}


	static SkeletonData::Joint convert(const MeshFile::Joint& joint){
		SkeletonData::Joint result;
		result.mInvBindPose = float4x4(joint.data);
		result.mNameSid = hash(joint.name);
		result.mParent = joint.parent;
		return result;
	}


	struct SkeletonVertexAdjInfo : public SkeletonData::vertex_adj{
		SkeletonVertexAdjInfo(const MeshFile::SkeletonAdjInfo& info)
			:Vertex::SkeAdjInfo(info.indices, float3(info.weights)){
		}
		SkeletonVertexAdjInfo& operator=(const MeshFile::SkeletonAdjInfo& info){
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


	std::shared_ptr<SkeletonData> SkeletonData::Load(const std::wstring& fileName){
		auto fin = win::File::Open(fileName, win::File::TO_READ);
		MemoryChunk memory{
			leo::make_unique<stdex::byte[]>(min.GetSize()),
			min.GetSize()
		};
		min.Read(memory.mData.get(), min.GetSize(), 0);
		return SkeletonData::Load(memory);
	}
	//从内存载入,未实现
	std::shared_ptr<SkeletonData> SkeletonData::Load(const MemoryChunk& min){
		try{
			auto out = leo::make_shared<SkeletonData>();
			auto device = DeviceMgr().GetDevice();
			leo::MeshFile::MeshFileHeader l3d_header;
			std::uint64_t offset = 0;
			min.Read(&l3d_header, sizeof(l3d_header), offset);

			offset += sizeof(l3d_header);
			out->mSubSets.resize(l3d_header.numsubset);
			ReadSubSets(min, offset, out->mSubSets, device);
			std::vector<vertex> vertices(l3d_header.numvertice);
			ReadVertices(min, offset, vertices);
			std::vector<std::uint32_t> indices(l3d_header.numindex);
			ReadIndices(min, offset, indices);

			D3D11_BUFFER_DESC Desc;
			Desc.Usage = D3D11_USAGE_IMMUTABLE;
			Desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			Desc.CPUAccessFlags = 0;
			Desc.MiscFlags = 0;
			Desc.StructureByteStride = 0;
			Desc.ByteWidth = static_cast<win::uint> (sizeof(vertex)*vertices.size());

			D3D11_SUBRESOURCE_DATA resDesc;
			resDesc.pSysMem = &vertices[0];
			dxcall(device->CreateBuffer(&Desc, &resDesc, &out->mVertexBuffer));

			leo::MeshFile::SkeletonHeader ske_header;
			min.Read(&ske_header, sizeof(ske_header), offset);
			offset += sizeof(ske_header);

			if (ske_header.loop == 0xffu && ske_header.numanima == 0xffffffffu && ske_header.numjoint == 0xffffffffu){
				ReadLodIndices(min, offset, out->mSubSets, indices);

				min.Read(&ske_header, sizeof(ske_header), offset);
				offset += sizeof(ske_header);
			}
			Desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
			Desc.ByteWidth = static_cast<win::uint> (sizeof(std::uint32_t)*indices.size());
			resDesc.pSysMem = &indices[0];
			dxcall(device->CreateBuffer(&Desc, &resDesc, &out->mIndicesBuffer));


			std::vector<leo::MeshFile::SkeletonAdjInfo> adjinfos(l3d_header.numvertice);
			min.Read(&adjinfos[0], sizeof(MeshFile::SkeletonAdjInfo)*adjinfos.size(), offset);
			offset += sizeof(MeshFile::SkeletonAdjInfo)*adjinfos.size();

			std::vector<SkeletonVertexAdjInfo> animationdatas(adjinfos.begin(), adjinfos.end());
			CD3D11_BUFFER_DESC aniVbdesc(sizeof(SkeletonVertexAdjInfo)*animationdatas.size(), D3D11_BIND_VERTEX_BUFFER, D3D11_USAGE_IMMUTABLE);
			D3D11_SUBRESOURCE_DATA aniSubDesc;
			aniSubDesc.pSysMem = animationdatas.data();
			dxcall(device->CreateBuffer(&aniVbdesc, &aniSubDesc, &out->mAnimationDataBUffer));


			std::vector<MeshFile::Joint> joints(ske_header.numjoint);
			min.Read(&joints[0], sizeof(MeshFile::Joint)*joints.size(), offset);
			offset += sizeof(MeshFile::Joint)*joints.size();



			std::vector<MeshFile::AnimatClip> anima(ske_header.numanima);
			for (auto & data : anima)
				data.data = make_unique<MeshFile::JointAnimaSample[]>(ske_header.numjoint);

			//Todo:
			auto & mSkeleton = out->mSkeleton;
			mSkeleton.mJointCount = ske_header.numjoint;


			mSkeleton.mJoints = std::make_unique<Joint[]>(mSkeleton.mJointCount);

			for (auto i = 0u; i != mSkeleton.mJointCount; ++i)
				mSkeleton.mJoints[i] = convert(joints[i]);

			AnimationClip mClip;

			wchar_t name[260];
			std::uint32_t numframe;
			for (auto & clip : anima){
				//读入名字和帧数量
				min.Read(name, sizeof(name), offset);
				offset += sizeof(name);
				min.Read(&numframe, sizeof(uint32), offset);
				offset += sizeof(uint32);

				//分配时间内存并读入
				clip.timedata = std::make_unique<float[]>(numframe);
				min.Read(clip.timedata.get(), sizeof(float)*numframe, offset);
				offset += sizeof(float)*numframe;

				//写入部分信息
				mClip.mFCount = numframe - 1;
				mClip.mLoop = true;

				//为读入分配足够内存
				for (auto j = 0u; j != ske_header.numjoint; ++j)
					clip.data[j].data = std::make_unique<SeqSQT[]>(numframe);
				//开始读入
				for (auto j = 0u; j != ske_header.numjoint; ++j){
					min.Read(clip.data[j].data.get(), sizeof(SeqSQT)*numframe, offset);
					offset += sizeof(SeqSQT)*numframe;
				}



				//转换读入SampleInfo至AnimationSample
				//1.分配内存
				//2.写入时间信息
				mClip.mSamples = std::make_unique<AnimationSample[]>(numframe);
				for (auto f = 0u; f != numframe; ++f){
					mClip.mSamples[f].mJointsPose = std::make_unique<JointPose[]>(mSkeleton.mJointCount);
					mClip.mSamples[f].mTimePoint = clip.timedata[f];
				}

				convertAnimaSample(mClip.mSamples, clip.data, numframe, mSkeleton.mJointCount);

				auto sid = hash(name);
				out->mAnimaNames.push_back(sid);
				out->mAnimations.emplace(sid, std::move(mClip));
			}
		}
		catch (leo::win::dx_exception & e)
		{
			OutputDebugStringA(e.what());
			return false;
		}
	}

}