//
//  Leo Engine Source File.
//  Copyright (C), FNS Studios, 2014-2014.
// -------------------------------------------------------------------------
//  File name:   Core/Terrain.h
//  Version:     v1.00
//  Created:     8/29/2014 by leo hawke.
//  Compilers:   Visual Studio.NET 2013
//  Description: 简易地形渲染系统<简易LOD>
// -------------------------------------------------------------------------
//  History:
//				
//
////////////////////////////////////////////////////////////////////////////

#ifndef Core_Terrain_hpp
#define Core_Terrain_hpp



#include <platform.h>
#include <ConstexprMath.hpp>
#include <leoint.hpp>
#include <LeoMath.h>
#include <memory.hpp>
#include <Tree.hpp>
#include "Camera.hpp"
#include "Vertex.hpp"
#include "..\exception.hpp"
#include "..\DeviceMgr.h"
#include "..\TextureMgr.h"
#include "..\ShaderMgr.h"
#include "EffectTerrain.hpp"
#include "EffectTerrainSO.hpp"
#include "..\file.hpp"
#include <vector>


namespace leo
{
	
	template<std::uint8_t MAXLOD = 4, std::uint16_t MAXEDGE = 128, std::uint8_t MINEDGEPIXEL = 6>
	//static_assert(pow(2,MAXLOD) <= MAXEDGE)
	class Terrain : public LodAlloc
	{
	public:
		//TerrainFile fomrat:
		//struct TerrainFileHeader
		//{
		//float mChunkSize;
		//std::uint32_t mHorChunkNum;
		//std::uint32_t mVerChunkNum;
		//std::wchar_t mHeightMap[file::max_path];
		//}
		Terrain(ID3D11Device* device, const std::wstring& terrainfilename)
		{
			struct TerrainFileHeader
			{
				float mChunkSize;
				std::uint32_t mHorChunkNum;
				std::uint32_t mVerChunkNum;
				wchar_t mHeightMap[leo::win::file::max_path];
			}mTerrainFileHeader;
			auto pFile = leo::win::File::Open(terrainfilename, win::File::TO_READ | win::File::NO_CREATE);
			pFile->Read(&mTerrainFileHeader, sizeof(TerrainFileHeader), 0);
			mHorChunkNum = mTerrainFileHeader.mHorChunkNum;
			mVerChunkNum = mTerrainFileHeader.mVerChunkNum;
			mChunkSize = mTerrainFileHeader.mChunkSize;

			mChunksQuadTree.Reset(float4(0.f, 0.f,
				mHorChunkNum*mChunkSize,
				mVerChunkNum*mChunkSize));

			for (auto slotX = 0; slotX != mHorChunkNum; ++slotX)
			{
				for (auto slotY = 0; slotY != mVerChunkNum; ++slotY)
				{
					mChunksQuadTree.Insert(Chunk(slotX, slotY), Offset(slotX,slotY));
				}
			}
			//Create VertexBuffer
			auto beginX = -mTerrainFileHeader.mChunkSize / 2;
			auto beginY = mTerrainFileHeader.mChunkSize / 2;
			auto delta = mTerrainFileHeader.mChunkSize / (MAXEDGE);
			for (auto slotY = 0; slotY <= MAXEDGE; ++slotY)
			{
				auto y = beginY - slotY*delta;
				for (auto slotX = 0; slotX <= MAXEDGE; ++slotX)
				{
					auto x = beginX + slotX*delta;
					mVertexs[slotY*(MAXEDGE + 1) + slotX] = half2(x, y);
				}
			}
			try {
				CD3D11_BUFFER_DESC vbDesc(sizeof(Vertex)*mVertexs.size(), D3D11_BIND_VERTEX_BUFFER, D3D11_USAGE_IMMUTABLE);
				D3D11_SUBRESOURCE_DATA vbDataDesc = { vbDataDesc.pSysMem = mVertexs.data(), 0, 0 };

				dxcall(device->CreateBuffer(&vbDesc, &vbDataDesc, &mCommonVertexBuffer));

				CD3D11_BUFFER_DESC sovbDesc(8*(MAXEDGE + 1)*(MAXEDGE + 1), D3D11_BIND_STREAM_OUTPUT, D3D11_USAGE_DEFAULT);
				dxcall(device->CreateBuffer(&sovbDesc, nullptr, &mSOTargetBuffer));

				CD3D11_BUFFER_DESC readvbDesc(8*(MAXEDGE + 1)*(MAXEDGE + 1),0, D3D11_USAGE_STAGING, D3D11_CPU_ACCESS_READ);
				dxcall(device->CreateBuffer(&readvbDesc, nullptr, &mReadBuffer));
			}
			Catch_DX_Exception

				//Create IndexBuffer
				std::vector<std::uint32_t> mIndexs{};
			std::size_t mIndexCount = 0;
			std::size_t mLodCount = MAXEDGE*MAXEDGE * 6;
			std::size_t mCrackCount = MAXEDGE / 2 * 3 * 4;
			for (auto slotLod = 0; slotLod != MAXLOD + 1; ++slotLod)
			{
				mIndexCount += mLodCount;
				mLodCount >>= 2;
				if (slotLod != MAXLOD)
					mIndexCount += mCrackCount;
				mCrackCount >>= 1;
			}
			mIndexs.reserve(mIndexCount);
			for (auto slotLod = 0; slotLod != MAXLOD + 1; ++slotLod) {
				auto powdelta = static_cast<std::uint16_t>(std::pow(2, slotLod));

				mIndexInfo[slotLod].mOffset = mIndexs.size();
				for (auto slotY = 0; slotY < MAXEDGE;) {
					for (auto slotX = 0; slotX < MAXEDGE;)
					{
						auto baseIndex = static_cast<std::uint16_t>(slotY*(MAXEDGE + 1) + slotX);
						auto xdelta = powdelta;
						//if (slotX + xdelta > MAXEDGE)
							//xdelta -= (slotX + xdelta+1 - MAXEDGE);
						auto ydelta = powdelta;
						//if (slotY + ydelta > MAXEDGE)
							//ydelta -= (slotY + ydelta + 1 - MAXEDGE);

						mIndexs.emplace_back(baseIndex);
						mIndexs.emplace_back(baseIndex + xdelta);
						mIndexs.emplace_back(baseIndex + ydelta*(MAXEDGE + 1));

						mIndexs.emplace_back(baseIndex + xdelta);
						mIndexs.emplace_back(baseIndex + ydelta*(MAXEDGE + 1) + xdelta);
						mIndexs.emplace_back(baseIndex + ydelta*(MAXEDGE + 1));

						slotX += powdelta;
					}
					slotY += powdelta;
				}
				mIndexInfo[slotLod].mCount = mIndexs.size() - mIndexInfo[slotLod].mOffset;
				mIndexInfo[slotLod].mCrackOffset[(uint8)DIRECTION_2D_TYPE::DIRECTION_LEFT] = mIndexs.size();
				if (slotLod != MAXLOD)
				{
					for (auto slotY = 0; slotY < MAXEDGE;)
					{
						auto baseIndex = slotY* (MAXEDGE + 1);
						mIndexs.emplace_back(baseIndex);
						slotY += powdelta;
						baseIndex = slotY* (MAXEDGE + 1);
						mIndexs.emplace_back(baseIndex);
						slotY += powdelta;
						baseIndex = slotY* (MAXEDGE + 1);
						mIndexs.emplace_back(baseIndex);
					}
				}
				mIndexInfo[slotLod].mCrackCount[(uint8)DIRECTION_2D_TYPE::DIRECTION_LEFT] = mIndexs.size() - mIndexInfo[slotLod].mCrackOffset[(uint8)DIRECTION_2D_TYPE::DIRECTION_LEFT];

				mIndexInfo[slotLod].mCrackOffset[(uint8)DIRECTION_2D_TYPE::DIRECTION_RIGHT] = mIndexs.size();
				if (slotLod != MAXLOD)
				{
					for (auto slotY = 0; slotY < MAXEDGE;)
					{
						auto baseIndex = slotY* (MAXEDGE + 1) + MAXEDGE;
						mIndexs.emplace_back(baseIndex);
						slotY += powdelta;
						baseIndex = slotY* (MAXEDGE + 1) + MAXEDGE;
						mIndexs.emplace_back(baseIndex);
						slotY += powdelta;
						baseIndex = slotY* (MAXEDGE + 1) + MAXEDGE;
						mIndexs.emplace_back(baseIndex);
					}
				}
				mIndexInfo[slotLod].mCrackCount[(uint8)DIRECTION_2D_TYPE::DIRECTION_RIGHT] = mIndexs.size() - mIndexInfo[slotLod].mCrackOffset[(uint8)DIRECTION_2D_TYPE::DIRECTION_RIGHT];

				mIndexInfo[slotLod].mCrackOffset[(uint8)DIRECTION_2D_TYPE::DIRECTION_TOP] = mIndexs.size();
				if (slotLod != MAXLOD)
				{
					for (auto slotX = 0; slotX < MAXEDGE;)
					{
						auto baseIndex = slotX;
						mIndexs.emplace_back(baseIndex);
						slotX += powdelta;
						baseIndex = slotX;
						mIndexs.emplace_back(baseIndex);
						slotX += powdelta;
						baseIndex = slotX;
						mIndexs.emplace_back(baseIndex);
					}
				}
				mIndexInfo[slotLod].mCrackCount[(uint8)DIRECTION_2D_TYPE::DIRECTION_TOP] = mIndexs.size() - mIndexInfo[slotLod].mCrackOffset[(uint8)DIRECTION_2D_TYPE::DIRECTION_TOP];

				mIndexInfo[slotLod].mCrackOffset[(uint8)DIRECTION_2D_TYPE::DIRECTION_BOTTOM] = mIndexs.size();
				if (slotLod != MAXLOD)
				{
					for (auto slotX = 0; slotX < MAXEDGE;)
					{
						auto baseIndex = MAXEDGE*(MAXEDGE + 1) + slotX;
						mIndexs.emplace_back(baseIndex);
						slotX += powdelta;
						baseIndex = MAXEDGE*(MAXEDGE + 1) + slotX;
						mIndexs.emplace_back(baseIndex);
						slotX += powdelta;
						baseIndex = MAXEDGE*(MAXEDGE + 1) + slotX;
						mIndexs.emplace_back(baseIndex);
					}
				}
				mIndexInfo[slotLod].mCrackCount[(uint8)DIRECTION_2D_TYPE::DIRECTION_BOTTOM] = mIndexs.size() - mIndexInfo[slotLod].mCrackOffset[(uint8)DIRECTION_2D_TYPE::DIRECTION_BOTTOM];
			}
			try {
				CD3D11_BUFFER_DESC ibDesc(sizeof(std::uint32_t)*mIndexs.size(), D3D11_BIND_INDEX_BUFFER, D3D11_USAGE_IMMUTABLE);
				D3D11_SUBRESOURCE_DATA ibDataDesc = { mIndexs.data(), 0, 0 };

				dxcall(device->CreateBuffer(&ibDesc, &ibDataDesc, &mCommonIndexBuffer));
			}
			Catch_DX_Exception
			leo::TextureMgr tm;
			mNoiseMap = tm.LoadTextureSRV(mTerrainFileHeader.mHeightMap);

			mWeightMap = tm.LoadTextureSRV(L"Resource/blend.dds");
			mMatArrayMap = tm.LoadTexture2DArraySRV(std::array<const wchar_t*, 5>({
				L"Resource/grass.dds", 
				L"Resource/darkdirt.dds", 
				L"Resource/stone.dds",
				L"Resource/lightdirt.dds",
				L"Resource/snow.dds"}));

			{
				const uint16 size[] = { 256,512,1024,2048,4096 };

				auto clamp_size = [&](uint16 ord_size){
					uint8 i = 0;
					for (;i != arrlen(size);){
						if (ord_size > size[i])
							++i;
						else
							break;
					}
					i = std::min<size_t>(i, arrlen(size) - 1);
					return size[i];
				};

				mNormaMapHorSize = clamp_size(uint16(mHorChunkNum*MAXEDGE));
				mNormaMapVerSize = clamp_size(uint16(mVerChunkNum* MAXEDGE));

				float4 Params{ 1.f / mNormaMapHorSize,1.f / mNormaMapVerSize,5.f,5.f };

				D3D11_BUFFER_DESC Desc;
				Desc.Usage = D3D11_USAGE_DEFAULT;
				Desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
				Desc.CPUAccessFlags = 0;
				Desc.MiscFlags = 0;
				Desc.StructureByteStride = 0;
				Desc.ByteWidth = sizeof(Params);

				D3D11_SUBRESOURCE_DATA Data;
				Data.pSysMem = &Params;

				dxcall(device->CreateBuffer(&Desc, &Data, &mCHMCSCB));


				D3D11_TEXTURE2D_DESC NormalMapTexDesc;

				NormalMapTexDesc.Format = DXGI_FORMAT_R32_FLOAT;

				NormalMapTexDesc.ArraySize = 1;
				NormalMapTexDesc.MipLevels = 1;

				NormalMapTexDesc.SampleDesc.Count = 1;
				NormalMapTexDesc.SampleDesc.Quality = 0;

				NormalMapTexDesc.Width = mNormaMapHorSize;
				NormalMapTexDesc.Height =mNormaMapVerSize;

				NormalMapTexDesc.Usage = D3D11_USAGE_DEFAULT;
				NormalMapTexDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
				NormalMapTexDesc.CPUAccessFlags = 0;
				NormalMapTexDesc.MiscFlags = 0;

				auto mTex = leo::win::make_scope_com<ID3D11Texture2D>();

				leo::dxcall(device->CreateTexture2D(&NormalMapTexDesc, nullptr, &mTex));
				leo::dxcall(device->CreateShaderResourceView(mTex, nullptr, &mHeightMapSRV));
				leo::dxcall(device->CreateUnorderedAccessView(mTex, nullptr, &mHeightMapUAV));

				leo::ShaderMgr SM;
				mCHMCS = SM.CreateComputeShader(FileSearch::Search(EngineConfig::ShaderConfig::GetShaderFileName(L"terrain", D3D11_COMPUTE_SHADER)));

				leo::RenderStates SS;
				mSS = SS.GetSamplerState(L"NearestRepeat");
			}
		}
		~Terrain()
		{
			leo::win::ReleaseCOM(mCommonVertexBuffer);
			leo::win::ReleaseCOM(mCommonIndexBuffer);
			leo::win::ReleaseCOM(mSOTargetBuffer);
			leo::win::ReleaseCOM(mReadBuffer);
		}
#ifdef LB_IMPL_MSCPP
		static_assert(leo::constexprmath::pow<2, MAXLOD>::value < MAXEDGE, "The n LOD's edgevertex is 2 times (n+1) LOD's edgevertex");
#else
		static_assert(leo::constexprmath::pow<2, MAXLOD>() < MAXEDGE, "The n LOD's edgevertex is 2 times (n+1) LOD's edgevertex");
#endif
		static_assert(MAXLOD < 8, "Too Large LOD result in 'out of memory'");


	public:
		void Render(ID3D11DeviceContext* context, const Camera& camera)
		{
			static bool has_normal_map = false;
			if (!has_normal_map) {
				ComputerNormalMap(context);
				//has_normal_map = true;
			}

			context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			UINT strides[] = { sizeof(Terrain::Vertex) };
			UINT offsets[] = { 0 };
			context->IASetVertexBuffers(0, 1, &mCommonVertexBuffer, strides, offsets);
			context->IASetInputLayout(ShaderMgr().CreateInputLayout(InputLayoutDesc::Terrain));
			context->IASetIndexBuffer(mCommonIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

			auto & mEffect = EffectTerrain::GetInstance();
			mEffect->ViewProjMatrix(camera.ViewProj());
			mEffect->HeightMap(mHeightMapSRV);
			mEffect->UVScale(float2(1.f / mHorChunkNum / mChunkSize, 1.f / mVerChunkNum / mChunkSize));
			mEffect->WeightMap(mWeightMap);
			mEffect->MatArrayMap(mMatArrayMap);
			mEffect->Apply(context);

			mNeedDrawChunks.clear();

			DetermineDrawChunk(camera);


			for (auto chunk : mNeedDrawChunks) {
				auto slotX = chunk->mSlotX;
				auto slotY = chunk->mSlotY;
				auto offset = load(float4(slotX*mChunkSize, 0, slotY*-mChunkSize, 1.f));

				auto topleft = load(float3((mHorChunkNum)*mChunkSize / -2.f, 0, +(mVerChunkNum)*mChunkSize / 2));
				auto topright = load(float3((mHorChunkNum - 2)*mChunkSize / -2.f, 0, +(mVerChunkNum)*mChunkSize / 2));
				auto buttomleft = load(float3((mHorChunkNum)*mChunkSize / -2.f, 0, +(mVerChunkNum - 2)*mChunkSize / 2));

				auto lodlevel = chunk->mLodLevel = DetermineLod(Add( topleft , offset),Add( topright ,offset), load(camera.View()),load(camera.Proj()));

				mEffect->WorldOffset(Offset(slotX,slotY), context);
#ifdef DEBUG
				static std::array<float4, 256> mLodColor;
				static bool has_call = false;

				auto init_lod_color = [&]()
				{
					mLodColor[0] = float4(1.f, 0.f, 0.f, 1.f);
					mLodColor[1] = float4(0.f, 1.f, 0.f, 1.f);
					mLodColor[2] = float4(0.f, 0.f, 1.f, 1.f);
					mLodColor[3] = float4(1.f, 1.f, 0.f, 1.f);
					mLodColor[4] = float4(1.f, 1.f, 1.f, 1.f);
				};
				leo::call_once(has_call, init_lod_color);

				mEffect->LodColor(mLodColor[lodlevel], context);
#endif
				context->DrawIndexed(mIndexInfo[lodlevel].mCount, mIndexInfo[lodlevel].mOffset, 0);

				if (slotX > 0)//Left
				{
					DrawCrack(*chunk, Chunk(slotX - 1, slotY), DIRECTION_2D_TYPE::DIRECTION_LEFT, context);
				}
				if (slotX < mHorChunkNum - 1)//Right
				{
					DrawCrack(*chunk, Chunk(slotX + 1, slotY), DIRECTION_2D_TYPE::DIRECTION_RIGHT, context);
				}
				if (slotY > 0)//Top
				{
					DrawCrack(*chunk, Chunk(slotX, slotY - 1), DIRECTION_2D_TYPE::DIRECTION_TOP, context);
				}
				if (slotY < mVerChunkNum - 1)//Bottom
				{
					DrawCrack(*chunk, Chunk(slotX, slotY + 1), DIRECTION_2D_TYPE::DIRECTION_BOTTOM, context);
				}
			}
		}

		
		void CastShadow(ID3D11DeviceContext* context) {
			//Todo
			//GS生成,绘制阴影
		}

		float GetHeight(const float2& xz) const{
			//初始化一些东西
			static auto mScreenSize = DeviceMgr().GetClientSize();
			static auto mAveChunkNum = static_cast<uint32>((mScreenSize.first*mScreenSize.second) / (MINEDGEPIXEL*3.f*MAXEDGE*MAXEDGE))+3u;
			static std::map < std::pair<uint32, uint32>, std::vector<float>> mMap;
			static auto mSizePerEDGE = 1.f*mChunkSize / MAXEDGE;
			auto base = Offset(0, 0);
			base.x -= (mChunkSize / 2.f);
			base.y += (mChunkSize / 2.f);
			auto slotX = static_cast<uint32>((xz.x - base.x)/mChunkSize);
			auto slotY = static_cast<uint32>((base.y - xz.y) /mChunkSize);
			auto chunk = std::make_pair(slotX,slotY);
			if (mMap.find(chunk) == mMap.cend()) {
				if (mMap.size() > mAveChunkNum)
					mMap.erase(mMap.begin());
				//流输出
				auto context = DeviceMgr().GetDeviceContext();
				context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
				UINT strides[] = { sizeof(Terrain::Vertex) };
				UINT offsets[] = { 0 };
				context->IASetVertexBuffers(0, 1, &mCommonVertexBuffer, strides, offsets);
				context->IASetInputLayout(ShaderMgr().CreateInputLayout(InputLayoutDesc::Terrain));

				auto & mEffect = EffectTerrainSO::GetInstance();
				mEffect->HeightMap(mHeightMap);
				mEffect->UVScale(float2(1.f / mHorChunkNum / mChunkSize, 1.f / mVerChunkNum / mChunkSize));
				mEffect->WorldOffset(Offset(slotX, slotY));
				static UINT pOffsets[1] = { 0 };
				mEffect->SetSOTargets(1, &mSOTargetBuffer, pOffsets);


				mEffect->Apply(context);

				context->Draw((MAXEDGE + 1)*(MAXEDGE + 1), 0);

				//拷贝
				context->CopyResource(mReadBuffer, mSOTargetBuffer);
				//映射,读取
				std::vector<SOOutput> mHeights;
				D3D11_MAPPED_SUBRESOURCE mapSubRes;
				try {
					dxcall(context->Map(mReadBuffer, 0, D3D11_MAP_READ, 0, &mapSubRes));
					std::transform(
						(SOOutput*)mapSubRes.pData,
						((SOOutput*)mapSubRes.pData) + (MAXEDGE + 1)*(MAXEDGE + 1),
						std::back_insert_iterator<std::vector<SOOutput>>(mHeights),
						[](const SOOutput& data)
					{
						return data;
					}
					);
					context->Unmap(mReadBuffer, 0);
				}
				Catch_DX_Exception
				std::sort(mHeights.begin(), mHeights.end());
				std::vector<float> mY;
				std::transform(mHeights.begin(),mHeights.end(),std::back_insert_iterator<std::vector<float>>(mY),
					[](const SOOutput& data) {
					return data.H;
				});
				mMap.emplace(chunk,std::move(mY));
			}
			
			auto offset = Offset(slotX, slotY);
			offset.x -= (mChunkSize / 2.f);
			offset.y += (mChunkSize / 2.f);
			std::pair<uint32, uint32> mSamples[4];
			//暂时忽略有可能出现的数值错误,比如位于一个Chunk的边
			mSamples[0].first = static_cast<uint32>(((xz.x - offset.x) / mChunkSize) * MAXEDGE);
			mSamples[0].second = static_cast<uint32>(((offset.y - xz.y) / mChunkSize)*MAXEDGE);

			mSamples[1].first = mSamples[0].first + 1;
			mSamples[1].second = mSamples[0].second;

			mSamples[2].first = mSamples[0].first;
			mSamples[2].second = mSamples[0].second + 1;

			mSamples[3].first = mSamples[0].first + 1;
			mSamples[3].second = mSamples[0].second + 1;

			//水平和竖直上的插值
			auto xt = ((xz.x - offset.x) - mSamples[0].first*mSizePerEDGE) / mSizePerEDGE;
			auto yt = ((offset.y - xz.y) - mSamples[0].second*mSizePerEDGE) / mSizePerEDGE;

			return Lerp(
				Lerp(
				mMap[chunk][mSamples[0].second*(MAXEDGE + 1) + mSamples[0].first],
				mMap[chunk][mSamples[1].second*(MAXEDGE + 1) + mSamples[1].first],
				xt),
				Lerp(
				mMap[chunk][mSamples[2].second*(MAXEDGE + 1) + mSamples[2].first],
				mMap[chunk][mSamples[3].second*(MAXEDGE + 1) + mSamples[3].first],
				xt),
				yt);
		}
	private:
		struct SOOutput {
			union
			{
				struct {
					float H;
					uint32 Id;
				};
				uint64 data;
			};

			bool operator<(const SOOutput& rhs) {
				return Id < rhs.Id;
			}
		};

		struct Vertex
		{
			Vertex()
				:pos(0.f,0.f)
			{}
			Vertex& operator=(const half2 rhs)
			{
				pos = rhs;
				return *this;
			}
			half2 pos;
		};

		class Chunk
		{
		public:
			//row0 : [0.MAXEDGE)
			explicit Chunk(std::uint8_t lodlevel)
				:mLodLevel(lodlevel)
			{}
			Chunk()
			{}

			Chunk(std::uint32_t slotX, std::uint32_t slotY)
				:mSlotX(slotX), mSlotY(slotY) {
			}
			std::uint8_t mLodLevel = MAXLOD / 2;

			//自索引,用户绘制Crack
			std::uint32_t mSlotX;
			std::uint32_t mSlotY;

			bool operator==(const Chunk& chunk) const{
				return mSlotX == chunk.mSlotX && mSlotY == chunk.mSlotY;
			}
		};
		std::array<Vertex, (MAXEDGE + 1)*(MAXEDGE + 1)> mVertexs;

		float mChunkSize;
		std::uint32_t mHorChunkNum;
		std::uint32_t mVerChunkNum;

		uint16 mNormaMapHorSize;
		uint16 mNormaMapVerSize;

		ID3D11ShaderResourceView* mNoiseMap;

		ID3D11ShaderResourceView* mWeightMap;
		ID3D11ShaderResourceView* mMatArrayMap;
		ID3D11Buffer* mCommonVertexBuffer = nullptr;
		ID3D11Buffer* mCommonIndexBuffer = nullptr;

		ID3D11Buffer* mSOTargetBuffer = nullptr;
		ID3D11Buffer* mReadBuffer = nullptr;
		struct Index
		{
			std::uint32_t mOffset;
			std::uint32_t mCount;
			std::uint32_t mCrackOffset[DIRECTION_2D_TYPE::DIRECTIONS_2DS];
			std::uint32_t mCrackCount[DIRECTION_2D_TYPE::DIRECTIONS_2DS];
		};
		std::array<Index, MAXLOD + 1> mIndexInfo;

		std::pair<uint16, uint16> mScreenSize;

		QuadTree<Chunk> mChunksQuadTree;
		std::vector<Chunk*> mNeedDrawChunks;

		std::vector<float3, aligned_alloc<float3, 16>> mVersInfo;

		ID3D11ComputeShader* mCHMCS = nullptr;

		ID3D11SamplerState* mSS = nullptr;

		leo::win::unique_com<ID3D11ShaderResourceView> mHeightMapSRV = nullptr;
		leo::win::unique_com<ID3D11UnorderedAccessView> mHeightMapUAV = nullptr;

		leo::win::unique_com<ID3D11Buffer> mCHMCSCB = nullptr;
	private:
#if 0
		uint8 ClipToScreenSpaceLod(XMVECTOR clip0, XMVECTOR clip1)
		{
			mScreenSize = DeviceMgr().GetClientSize();

			clip0 = XMVectorDivide(clip0, XMVectorSplatW(clip0));
			clip1 = XMVectorDivide(clip1, XMVectorSplatW(clip1));


			float2 p0, p1;
			save(p0, clip0);
			save(p1, clip1);

			auto in_ndc = [](const float2& p)
			{
				return p.x >= -1.f && p.x <= 1.f && p.y >= -1.f && p.y <= 1.f;
			};


			if (in_ndc(p0) && in_ndc(p1))
			{
				goto allin;
			}
			if (in_ndc(p0)) {
				p1.x = 1.f;
			}
			else if (in_ndc(p1)) {
				p0.x = -1.f;
			}
			else
				return MAXLOD;

			clip0 = load(p0);
			clip1 = load(p1);

			DebugPrintf("%d %d\n", x, y);
			assert(p0.x <= p1.x);

		allin:
			auto gScreenSize = load(float4(mScreenSize, 1.f, 1.f));
			clip0 *= gScreenSize;
			clip1 *= gScreenSize;

			float d = XMVectorGetX(XMVector2Length(clip0 - clip1)) / MINEDGEPIXEL;

			//d /= 2;
			uint8 Lod = 0;

			for (; Lod < MAXLOD; ++Lod)
			{
				if (d > ((MAXEDGE >> Lod) - (MAXEDGE >> (Lod + 2))))
					break;
			}

			return Lod;
		}

		uint8 EdgeToScreenSpaceLod(XMVECTOR p0, XMVECTOR p1)
		{
			return ClipToScreenSpaceLod(p0, p1);
		}
#endif
		uint8 LM_VECTOR_CALL DistanceToCameraLod(__m128 p0, __m128 p1)
		{
			auto center = Divide(Add(p0, p1), Splat(2.f));
			auto distance = _mm_cvtss_f32(Length<3>(center));
			auto lod = distance / mChunkSize;
			auto ilod = int(lod - 0.5f);
			if (ilod < 0)
				return 0;
			if (ilod > MAXLOD)
				return MAXLOD;
			return uint8(ilod);
		}

		uint8 LM_VECTOR_CALL DetermineLod(__m128 p0, __m128 p1,const std::array<__m128,4>& view, const std::array<__m128, 4>& proj)
		{
			auto vp0 = Transform(p0, view);
			auto vp1 = Transform(p1, view);
			auto cameralod = DistanceToCameraLod(vp0, vp1);

			//auto pp0 = XMVector4Transform(vp0, proj);
			//auto pp1 = XMVector4Transform(vp1, proj);
			//auto edgelod = EdgeToScreenSpaceLod(pp0, pp1);

			//auto lod = max(cameralod, MAXLOD);
			return cameralod;
		}

		void DrawCrack(const Chunk& chunkInfo, const Chunk& neigbourChunk, DIRECTION_2D_TYPE direct, ID3D11DeviceContext* context)
		{
			if (std::find_if(mNeedDrawChunks.cbegin(), mNeedDrawChunks.cend(), [&neigbourChunk](const Chunk* chunk) {
				return *chunk == neigbourChunk;
			}
				) != mNeedDrawChunks.cend())
				if (neigbourChunk.mLodLevel > chunkInfo.mLodLevel)

					context->DrawIndexed(mIndexInfo[chunkInfo.mLodLevel].mCrackCount[(uint8)direct], mIndexInfo[chunkInfo.mLodLevel].mCrackOffset[(uint8)direct],  0);
		}

		void DetermineDrawChunk(const Camera& camera)
		{
			static auto value_function = [](const Chunk& chunk, std::vector<Chunk*>& mNeedDrawChunks) {
				mNeedDrawChunks.push_back(const_cast<Chunk*>(&chunk));
			};
			static auto clip_function = [](const float4& rect, const Camera& camera) {
				float3 v[3];
				v[0] = float3(rect.x - rect.z / 2, 0, rect.y + rect.w / 2);
				v[1] = float3(rect.x + rect.z / 2, 0, rect.y + rect.w / 2);
				v[2] = float3(rect.x - rect.z / 2, 0, rect.y - rect.w / 2);

				return camera.Contains(Triangle(v)) != leo::CONTAINMENT_TYPE::DISJOINT;
			};

			static auto value_param = std::make_tuple(std::ref(mNeedDrawChunks));
			mChunksQuadTree.Iterator(
				value_function,
				value_param,
				clip_function,
				std::make_tuple(std::cref(camera))
				);
		}

		float2 Offset(uint32 slotX, uint32 slotY) const{
			return float2((mHorChunkNum - 1)*mChunkSize / -2.f + slotX*mChunkSize, +(mVerChunkNum - 1)*mChunkSize / 2 - slotY*mChunkSize);
		}

		void ComputerNormalMap(ID3D11DeviceContext* context) {
			ID3D11ShaderResourceView* nullSRV = nullptr;
			context->VSSetShaderResources(0, 1, &nullSRV);
			context->PSSetShaderResources(3, 1, &nullSRV);

			context->CSSetShader(mCHMCS, nullptr, 0);
			context->CSSetUnorderedAccessViews(0, 1, &mHeightMapUAV, nullptr);
			context->CSSetConstantBuffers(0, 1, &mCHMCSCB);
			context->CSSetShaderResources(0, 1, &mNoiseMap);
			context->CSSetSamplers(0, 1, &mSS);

			context->Dispatch(mNormaMapHorSize / 32, mNormaMapVerSize / 32, 1);

			ID3D11UnorderedAccessView* mNullptrUAV = nullptr;
			context->CSSetUnorderedAccessViews(0, 1, &mNullptrUAV, nullptr);
			context->CSSetShader(nullptr, nullptr, 0);

		}
	};
}
#endif