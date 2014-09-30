//
//  Leo Engine Source File.
//  Copyright (C), FNS Studios, 2014-2014.
// -------------------------------------------------------------------------
//  File name:   Core/Terrain.h[[
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

#include "..\IndePlatform\\ConstexprMath.hpp"
#include "..\IndePlatform\leoint.hpp"
#include "..\IndePlatform\LeoMath.h"
#include "..\exception.hpp"
#include "..\DeviceMgr.h"
#include "..\file.hpp"
#include <vector>


namespace leo
{
	template<std::uint8_t MAXLOD = 4,size_t MAXEDGEVERTEX = 256,size_t TRIWIDTH = 12>
	//static_assert(pow(2,MAXLOD) <= MAXEDGEVERTEX)
	class Terrain
	{
	public:
		//TerrainFile fomrat:
		//struct TerrainFileHeader
		//{
		//std::uint32_t mHorChunkNum;
		//std::uint32_t mVerChunkNum;
		//std::wchar_t mHeightMap[file::max_path];
		//}
		Terrain(ID3D11Device* device,const std::wstring& terrainfilename)
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
			mChunkVector.resize(mHorChunkNum*mTerrainFileHeader.mVerChunkNum);

			//Create VertexBuffer
			auto beginX = -mTerrainFileHeader.mChunkSize / 2;
			auto beginY = mTerrainFileHeader.mChunkSize / 2;
			auto delta = mTerrainFileHeader.mChunkSize / MAXEDGEVERTEX;
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
				CD3D11_BUFFER_DESC vbDesc(sizeof(Vertex)*mVertexs.size(),D3D11_BIND_VERTEX_BUFFER,D3D11_USAGE_IMMUTABLE);
				D3D11_SUBRESOURCE_DATA vbDataDesc = { vbDataDesc.pSysMem = mVertexs.data() ,0,0};
				
				dxcall(device->CreateBuffer(&vbDesc, &vbDataDesc, &mCommonVertexBuffer));
			}
			Catch_DX_Exception


			//Create IndexBuffer
			std::array<std::vector<std::uint16_t>, MAXLOD + 1> mIndexsArray;
			for (auto slotLod = 0; slotLod != MAXLOD + 1; ++slotLod){
				auto delta = static_cast<int>(std::pow(2, slotLod));
				for (auto slotY = 0; slotY < MAXEDGEVERTEX - 1;){
					for (auto slotX = 0; slotX < MAXEDGEVERTEX - 1;)
					{
						auto baseIndex = slotY*MAXEDGEVERTEX + slotX;
						mIndexsArray[slotLod].emplace_back(baseIndex);
						mIndexsArray[slotLod].emplace_back(baseIndex+delta);
						mIndexsArray[slotLod].emplace_back(baseIndex+delta*MAXEDGEVERTEX);

						mIndexsArray[slotLod].emplace_back(baseIndex+delta);
						mIndexsArray[slotLod].emplace_back(baseIndex+delta*MAXEDGEVERTEX+delta);
						mIndexsArray[slotLod].emplace_back(baseIndex+delta*MAXEDGEVERTEX);

						slotX += delta;
					}
					slotY += delta;
				}
			}
		}
#ifdef LB_IMPL_MSCPP
		static_assert(leo::constexprmath::pow<2, MAXLOD>::value <= MAXEDGEVERTEX, "The n LOD's edgevertex is 2 times (n+1) LOD's edgevertex");
#else
		static_assert(leo::constexprmath::pow<2, MAXLOD>() <= MAXEDGEVERTEX, "The n LOD's edgevertex is 2 times (n+1) LOD's edgevertex");
#endif
		static_assert(MAXLOD < 8, "Too Large LOD result in 'out of memory'");
	private:
		struct Vertex
		{
			Vertex& operator=(const half2 rhs)
			{
				pos = rhs;
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
			std::uint8_t mLodLevel = MAXLOD/2;
		};
		std::array<Vertex, MAXEDGEVERTEX*MAXEDGEVERTEX> mVertexs;
		
		std::vector<Chunk> mChunkVector;
		std::uint16_t mHorChunkNum;

		ID3D11ShaderResourceView* mHeightMap;
		ID3D11Buffer* mCommonVertexBuffer;
		std::array<ID3D11Buffer*, MAXLOD + 1> mIndexBuffer;
	};
}

#endif