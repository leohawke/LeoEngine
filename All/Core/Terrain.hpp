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
	template<std::uint8_t MAXLOD = 4, size_t MAXEDGEVERTEX = 256, size_t TRIWIDTH = 12>
	//static_assert(pow(2,MAXLOD) <= MAXEDGEVERTEX)
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
			auto delta = mTerrainFileHeader.mChunkSize / (MAXEDGEVERTEX-1);
			for (auto slotY = 0; slotY != MAXEDGEVERTEX; ++slotY)
			{
				auto y = beginY - slotY*delta;
				for (auto slotX = 0; slotX != MAXEDGEVERTEX; ++slotX)
				{
					auto x = beginX + slotX*delta;
					mVertexs[slotY*MAXEDGEVERTEX + slotX] = half2(x, y);
				}
			}
			try{
				CD3D11_BUFFER_DESC vbDesc(sizeof(Vertex)*mVertexs.size(), D3D11_BIND_VERTEX_BUFFER, D3D11_USAGE_IMMUTABLE);
				D3D11_SUBRESOURCE_DATA vbDataDesc = { vbDataDesc.pSysMem = mVertexs.data(), 0, 0 };

				dxcall(device->CreateBuffer(&vbDesc, &vbDataDesc, &mCommonVertexBuffer));
			}
			Catch_DX_Exception

				//Create IndexBuffer
				std::array<std::vector<std::uint32_t>, MAXLOD + 1> mIndexsArray;
			for (auto slotLod = 0; slotLod != MAXLOD + 1; ++slotLod){
				auto powdelta = static_cast<std::uint16_t>(std::pow(2, slotLod));
				for (auto slotY = 0; slotY < MAXEDGEVERTEX - 1;){
					for (auto slotX = 0; slotX < MAXEDGEVERTEX - 1;)
					{
						auto baseIndex = static_cast<std::uint16_t>(slotY*MAXEDGEVERTEX + slotX);
						auto xdelta = powdelta;
						if (slotX + xdelta > MAXEDGEVERTEX-1)
							xdelta -= (slotX + xdelta+1 - MAXEDGEVERTEX);
						auto ydelta = powdelta;
						if (slotY + ydelta > MAXEDGEVERTEX-1)
							ydelta -= (slotY + ydelta + 1 - MAXEDGEVERTEX);

						mIndexsArray[slotLod].emplace_back(baseIndex);
						mIndexsArray[slotLod].emplace_back(baseIndex + xdelta);
						mIndexsArray[slotLod].emplace_back(baseIndex + ydelta*static_cast<std::uint16_t>(MAXEDGEVERTEX));

						mIndexsArray[slotLod].emplace_back(baseIndex + xdelta);
						mIndexsArray[slotLod].emplace_back(baseIndex + ydelta*static_cast<std::uint16_t>(MAXEDGEVERTEX)+xdelta);
						mIndexsArray[slotLod].emplace_back(baseIndex + ydelta*static_cast<std::uint16_t>(MAXEDGEVERTEX));

						slotX += powdelta;
					}
					slotY += powdelta;
				}
				try{
					mIndexNum[slotLod] = mIndexsArray[slotLod].size();
					CD3D11_BUFFER_DESC ibDesc(sizeof(std::uint32_t)*mIndexsArray[slotLod].size(), D3D11_BIND_INDEX_BUFFER, D3D11_USAGE_IMMUTABLE);
					D3D11_SUBRESOURCE_DATA ibDataDesc = { mIndexsArray[slotLod].data(), 0, 0 };

					dxcall(device->CreateBuffer(&ibDesc, &ibDataDesc, &mIndexBuffer[slotLod]));
				}
				Catch_DX_Exception
			}

			leo::TextureMgr tm;
			mHeightMap = tm.LoadTextureSRV(mTerrainFileHeader.mHeightMap);
		}
		~Terrain()
		{
			leo::win::ReleaseCOM(mCommonVertexBuffer);
			for (auto & i : mIndexBuffer)
				leo::win::ReleaseCOM(i);
		}
#ifdef LB_IMPL_MSCPP
		static_assert(leo::constexprmath::pow<2, MAXLOD>::value <= MAXEDGEVERTEX, "The n LOD's edgevertex is 2 times (n+1) LOD's edgevertex");
#else
		static_assert(leo::constexprmath::pow<2, MAXLOD>() <= MAXEDGEVERTEX, "The n LOD's edgevertex is 2 times (n+1) LOD's edgevertex");
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

			auto & mEffect = EffectTerrain::GetInstance();
			mEffect->ViewProjMatrix(camera.ViewProj());
			mEffect->HeightMap(mHeightMap);
			mEffect->UVScale(float2(1.f / mHorChunkNum / mChunkSize, 1.f / mVerChunkNum / mChunkSize));
			mEffect->Apply(context);

			for (auto slotX = 0; slotX != mHorChunkNum; ++slotX)
			{
				for (auto slotY = 0; slotY != mVerChunkNum; ++slotY)
				{
					//auto offset = load(float3(slotX*mChunkSize, 0, -slotY*mChunkSize));
					auto offset = load(float4(slotX*mChunkSize, 0, -slotY*mChunkSize,1.f));
					//see calc topleft,topright,...
					if (camera.Contains(topleft + offset, topright + offset, buttomleft + offset))
					{
						float2 worldoffset(-(mHorChunkNum-1)*mChunkSize / 2 + slotX*mChunkSize, +(mVerChunkNum-1)*mChunkSize / 2-slotY*mChunkSize);
						mEffect->WorldOffset(worldoffset, context);

						auto lodlevel = mChunkVector[slotY*mHorChunkNum + slotX].mLodLevel = DetermineLod(buttomleft + offset, topright + offset, camera.View(),camera.Proj());

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

						mEffect->LodColor(mLodColor[lodlevel],context);
#endif

						context->IASetIndexBuffer(mIndexBuffer[lodlevel], DXGI_FORMAT_R32_UINT, 0);
						context->DrawIndexed(mIndexNum[lodlevel], 0, 0);
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
			//row0 : [0.MAXEDGEVERTEX)
			explicit Chunk(std::uint8_t lodlevel)
				:mLodLevel(lodlevel)
			{}
			Chunk()
			{}
			std::uint8_t mLodLevel = MAXLOD / 2;
		};
		std::array<Vertex, MAXEDGEVERTEX*MAXEDGEVERTEX> mVertexs;

		std::vector<Chunk> mChunkVector;
		float mChunkSize;
		std::uint16_t mHorChunkNum;
		std::uint16_t mVerChunkNum;

		ID3D11ShaderResourceView* mHeightMap;
		ID3D11Buffer* mCommonVertexBuffer;
		std::array<ID3D11Buffer*, MAXLOD + 1> mIndexBuffer;
		std::array<UINT, MAXLOD + 1> mIndexNum;

		std::pair<uint16,uint16> mScreenSize;
	private:
		uint8 ClipToScreenSpaceLod(XMVECTOR clip0, XMVECTOR clip1)
		{
			mScreenSize = DeviceMgr().GetClientSize();

			clip0 =XMVectorDivide(clip0,XMVectorSplatW(clip0));
			clip1 = XMVectorDivide(clip1, XMVectorSplatW(clip1));

			//const auto Vmin = XMVectorSet(-1.f, -1.f, 0.f, 0.f);
			//const auto Vmax = XMVectorSet(1.f, 1.f, 0.f, 0.f);

			//clip1 = XMVectorClamp(clip1, Vmin, Vmax);
			//clip0 = XMVectorClamp(clip0, Vmin, clip1);

			auto gScreenSize = load(float4(mScreenSize, 1.f, 1.f));
			clip0 *= gScreenSize;
			clip1 *= gScreenSize;

			float d = XMVectorGetX(XMVector2Length(clip0 - clip1)) / TRIWIDTH;

			uint8 Lod = 0;

			for (; Lod < MAXLOD;++Lod)
			{
				if (d >((MAXEDGEVERTEX >> Lod) - (MAXEDGEVERTEX >> (Lod+2))))
					break;
			}

			return Lod;
		}

		uint8 EdgeToScreenSpaceLod(XMVECTOR p0, XMVECTOR p1)
		{
			return ClipToScreenSpaceLod(p0, p1);
		}

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

			auto pp0 = XMVector4Transform(p0, proj);
			auto pp1 = XMVector4Transform(p1, proj);
			auto edgelod = EdgeToScreenSpaceLod(pp0, pp1);

			auto lod = min(cameralod, edgelod);
			return lod;
		}
	};
}

#endif