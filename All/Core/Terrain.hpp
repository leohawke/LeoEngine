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

#include "..\IndePlatform\platform.h"
#include "..\IndePlatform\\ConstexprMath.hpp"
#include "..\IndePlatform\leoint.hpp"
#include "..\IndePlatform\LeoMath.h"
#include "..\IndePlatform\memory.hpp"
#include "Camera.hpp"
#include "..\exception.hpp"
#include "..\DeviceMgr.h"
#include "..\TextureMgr.h"
#include "..\ShaderMgr.h"
#include "EffectTerrain.hpp"
#include "..\file.hpp"
#include <vector>


namespace leo
{
	namespace InputLayoutDesc
	{
		extern const D3D11_INPUT_ELEMENT_DESC Terrain[1];
	}
	template<std::uint8_t MAXLOD = 4, std::uint16_t MAXEDGE = 256, std::uint8_t MINEDGEPIXEL = 6>
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
			mChunkVector.resize(mHorChunkNum*mVerChunkNum);

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
					mVertexs[slotY*(MAXEDGE+1) + slotX] = half2(x, y);
				}
			}
			try{
				CD3D11_BUFFER_DESC vbDesc(sizeof(Vertex)*mVertexs.size(), D3D11_BIND_VERTEX_BUFFER, D3D11_USAGE_IMMUTABLE);
				D3D11_SUBRESOURCE_DATA vbDataDesc = { vbDataDesc.pSysMem = mVertexs.data(), 0, 0 };

				dxcall(device->CreateBuffer(&vbDesc, &vbDataDesc, &mCommonVertexBuffer));
			}
			Catch_DX_Exception

				//Create IndexBuffer
			std::vector<std::uint32_t> mIndexs {};
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
			for (auto slotLod = 0; slotLod != MAXLOD + 1; ++slotLod){
				auto powdelta = static_cast<std::uint16_t>(std::pow(2, slotLod));

				mIndexInfo[slotLod].mOffset = mIndexs.size();
				for (auto slotY = 0; slotY < MAXEDGE;){
					for (auto slotX = 0; slotX < MAXEDGE;)
					{
						auto baseIndex = static_cast<std::uint16_t>(slotY*(MAXEDGE+1) + slotX);
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
#if 0
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
						auto baseIndex = slotY* (MAXEDGE + 1)+MAXEDGE;
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
						auto baseIndex =MAXEDGE*(MAXEDGE+1) + slotX;
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
#endif
			}
			//assert(mIndexCount == mIndexs.size());
			try{
				CD3D11_BUFFER_DESC ibDesc(sizeof(std::uint32_t)*mIndexs.size(), D3D11_BIND_INDEX_BUFFER, D3D11_USAGE_IMMUTABLE);
				D3D11_SUBRESOURCE_DATA ibDataDesc = { mIndexs.data(), 0, 0 };

				dxcall(device->CreateBuffer(&ibDesc, &ibDataDesc, &mCommonIndexBuffer));
			}
			Catch_DX_Exception
			leo::TextureMgr tm;
			mHeightMap = tm.LoadTextureSRV(mTerrainFileHeader.mHeightMap);

			mWeightMap = tm.LoadTextureSRV(L"Resource\\weight.jpg");
			mMatArrayMap = tm.LoadTexture2DArraySRV(std::array<const wchar_t*, 3>({ L"Resource\\Grass.jpg", L"Resource\\Soil.jpg", L"Resource\\Snow.jpg" }));
		}
		~Terrain()
		{
			leo::win::ReleaseCOM(mCommonVertexBuffer);
			leo::win::ReleaseCOM(mCommonIndexBuffer);
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
			auto topleft = load(float3(-(mHorChunkNum)*mChunkSize / 2, 0, +(mVerChunkNum)*mChunkSize / 2));
			auto topright = load(float3(-(mHorChunkNum - 2)*mChunkSize / 2, 0, +(mVerChunkNum)*mChunkSize / 2));
			auto buttomleft = load(float3(-(mHorChunkNum)*mChunkSize / 2, 0, +(mVerChunkNum - 2)*mChunkSize / 2));

			context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			UINT strides[] = { sizeof(Terrain::Vertex) };
			UINT offsets[] = { 0 };
			context->IASetVertexBuffers(0, 1, &mCommonVertexBuffer, strides, offsets);
			context->IASetInputLayout(ShaderMgr().CreateInputLayout(InputLayoutDesc::Terrain));
			context->IASetIndexBuffer(mCommonIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

			auto & mEffect = EffectTerrain::GetInstance();
			mEffect->ViewProjMatrix(camera.ViewProj());
			mEffect->HeightMap(mHeightMap);
			mEffect->UVScale(float2(1.f / mHorChunkNum / mChunkSize, 1.f / mVerChunkNum / mChunkSize));
			mEffect->WeightMap(mWeightMap);
			mEffect->MatArrayMap(mMatArrayMap);
			mEffect->Apply(context);

			for (auto slotX = 0; slotX != mHorChunkNum; ++slotX)
			{
				for (auto slotY = 0; slotY != mVerChunkNum; ++slotY)
				{
					//auto offset = load(float3(slotX*mChunkSize, 0, -slotY*mChunkSize));
					auto offset = load(float4(slotX*mChunkSize, 0, -slotY*mChunkSize,1.f));
					auto chunkIndex = slotY*mHorChunkNum + slotX;
					//see calc topleft,topright,...
					if (camera.Contains(topleft + offset, topright + offset, buttomleft + offset))
					{
						mChunkVector[chunkIndex].mVisiable = true;
						auto lodlevel = mChunkVector[slotY*mHorChunkNum + slotX].mLodLevel = DetermineLod(topleft + offset, topright + offset, camera.View(), camera.Proj());

						float2 worldoffset(-(mHorChunkNum-1)*mChunkSize / 2 + slotX*mChunkSize, +(mVerChunkNum-1)*mChunkSize / 2-slotY*mChunkSize);
						mEffect->WorldOffset(worldoffset, context);

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
					}
					else
						mChunkVector[chunkIndex].mVisiable = true;
				}
			}
			for (auto slotX = 0; slotX != mHorChunkNum; ++slotX)
			{
				for (auto slotY = 0; slotY != mVerChunkNum; ++slotY)
				{
					auto chunkIndex = slotY*mHorChunkNum + slotX;
					if (mChunkVector[chunkIndex].mVisiable){
						//auto offset = load(float3(slotX*mChunkSize, 0, -slotY*mChunkSize));
						auto offset = load(float4(slotX*mChunkSize, 0, -slotY*mChunkSize, 1.f));
						auto chunkIndex = slotY*mHorChunkNum + slotX;
						float2 worldoffset(-(mHorChunkNum - 1)*mChunkSize / 2 + slotX*mChunkSize, +(mVerChunkNum - 1)*mChunkSize / 2 - slotY*mChunkSize);
						mEffect->WorldOffset(worldoffset, context);

						if (slotX > 0)//Left
						{
							auto nNeighbourIndex = chunkIndex - 1;
							DrawCrack(mChunkVector[chunkIndex], nNeighbourIndex, DIRECTION_2D_TYPE::DIRECTION_LEFT, context);
						}
						if (slotX < mHorChunkNum - 1)//Right
						{
							auto nNeighbourIndex = chunkIndex + 1;
							DrawCrack(mChunkVector[chunkIndex], nNeighbourIndex, DIRECTION_2D_TYPE::DIRECTION_RIGHT, context);
						}
						if (slotY > 0)//Top
						{
							auto nNeighbourIndex = chunkIndex - mHorChunkNum;
							DrawCrack(mChunkVector[chunkIndex], nNeighbourIndex, DIRECTION_2D_TYPE::DIRECTION_TOP, context);
						}
						if (slotY < mVerChunkNum - 1)//Bottom
						{
							auto nNeighbourIndex = chunkIndex + mHorChunkNum;
							DrawCrack(mChunkVector[chunkIndex], nNeighbourIndex, DIRECTION_2D_TYPE::DIRECTION_BOTTOM,context);
						}
					}
				}
			}
		}
	private:
		struct Vertex
		{
			Vertex()
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
			std::uint8_t mLodLevel = MAXLOD / 2;
			bool mVisiable = false;
		};
		std::array<Vertex, (MAXEDGE+1)*(MAXEDGE+1)> mVertexs;

		std::vector<Chunk> mChunkVector;
		float mChunkSize;
		std::uint16_t mHorChunkNum;
		std::uint16_t mVerChunkNum;

		ID3D11ShaderResourceView* mHeightMap;

		ID3D11ShaderResourceView* mWeightMap;
		ID3D11ShaderResourceView* mMatArrayMap;
		ID3D11Buffer* mCommonVertexBuffer;
		ID3D11Buffer* mCommonIndexBuffer;

		struct Index
		{
			std::uint32_t mOffset;
			std::uint32_t mCount;
			std::uint32_t mCrackOffset[DIRECTION_2D_TYPE::DIRECTIONS_2DS];
			std::uint32_t mCrackCount[DIRECTION_2D_TYPE::DIRECTIONS_2DS];
		};
		std::array<Index, MAXLOD + 1> mIndexInfo;

		std::pair<uint16,uint16> mScreenSize;
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
			if (in_ndc(p0)){
				p1.x = 1.f;
			}
			else if (in_ndc(p1)){
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

			for (; Lod < MAXLOD;++Lod)
			{
				if (d >((MAXEDGE >> Lod) - (MAXEDGE >> (Lod+2))))
					break;
			}

			return Lod;
		}

		uint8 EdgeToScreenSpaceLod(XMVECTOR p0, XMVECTOR p1)
		{
			return ClipToScreenSpaceLod(p0, p1);
		}
#endif
		uint8 DistanceToCameraLod(XMVECTOR p0, XMVECTOR p1)
		{
			auto center = XMVectorDivide(XMVectorAdd(p0, p1), XMVectorReplicate(2.f));
			auto distance = XMVectorGetX(XMVector3Length(center));
			auto lod = distance / mChunkSize;
			auto ilod = int(lod - 0.5f);
			if (ilod < 0)
				return 0;
			if (ilod > MAXLOD)
				return MAXLOD;
			return uint8(ilod);
		}

		uint8 DetermineLod(XMVECTOR p0, XMVECTOR p1, CXMMATRIX view,CXMMATRIX proj)
		{
			auto vp0 = XMVector4Transform(p0, view);
			auto vp1 = XMVector4Transform(p1, view);
			auto cameralod = DistanceToCameraLod(vp0, vp1);

			//auto pp0 = XMVector4Transform(vp0, proj);
			//auto pp1 = XMVector4Transform(vp1, proj);
			//auto edgelod = EdgeToScreenSpaceLod(pp0, pp1);

			//auto lod = max(cameralod, MAXLOD);
			return cameralod;
		}

		void DrawCrack(const Chunk& chunkInfo, std::size_t neigbourIndex, DIRECTION_2D_TYPE direct,ID3D11DeviceContext* context)
		{
			if (mChunkVector[neigbourIndex].mVisiable && mChunkVector[neigbourIndex].mLodLevel > chunkInfo.mLodLevel)
			{
				context->DrawIndexed(mIndexInfo[chunkInfo.mLodLevel].mCrackOffset[(uint8)direct], mIndexInfo[chunkInfo.mLodLevel].mCrackCount[(uint8)direct],0);
			}
		}
	};
}

#endif