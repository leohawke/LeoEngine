////////////////////////////////////////////////////////////////////////////
//
//  Leo Engine Source File.
//  Copyright (C), FNS Studios, 2014-2014.
// -------------------------------------------------------------------------
//  File name:   Core/Terrain.TileRing.h
//  Version:     v1.00
//  Created:     9/7/2014 by leo hawke.
//  Compilers:   Visual Studio.NET 2013
//  Description: 提供地形渲染的Tile环抽象,Tile是轴同像的,连续的,每个Ring使用不同
//				 的Tile/Patch大小构造不同的细节层次,实际上他们都是正方环,但是能够
//				 在抽象上表示为圆,内部最多的细节就是正方形,称之为退化环
// -------------------------------------------------------------------------
//  History:
//				
//
////////////////////////////////////////////////////////////////////////////

#ifndef Core_TileRing_h
#define Core_TileRing_h

#include "..\leomath.hpp"

struct ID3D11Device;
struct ID3D11DeviceContext;
struct ID3D11Buffer;
namespace leo
{
	namespace Vertex
	{
		struct InstanceData;
		struct Adjacency;
	}
	//Ctor
	//Int dimensions specified to the ctor in numbers of tiles
	//    <-   outerWidth  ->
	//    ###################
	//    ###################
	//    ###             ###
	//    ###<-holeWidth->###
	//    ###             ###
	//    ###    (0,0)    ###
	//    ###             ###
	//    ###             ###
	//    ###             ###
	//    ###################
	//    ###################
	class TileRing
	{
	public:
		// holeWidth & outerWidth are nos. of tiles
		// tileSize is a world-space length
		TileRing(ID3D11Device*, int holeWidth, int outerWidth, float tileSize);
		~TileRing();

		void SetRenderingState(ID3D11DeviceContext*) const;

		int   outerWidth() const { return mOuterWidth; }
		int   nTiles()     const { return mnTiles; }
		float tileSize()   const { return mtileSize; }

		TileRing(const TileRing&) = delete;
		TileRing& operator=(const TileRing&) = delete;
	private:
		void CreateInstanceDataVB(ID3D11Device*);
		bool InRing(int x, int y) const;
		void AssignNeighbourSizes(int x, int y,Vertex::Adjacency*) const;

		ID3D11Buffer* mPositionsVB;

		const int mHoleWidth, mOuterWidth, mRingWidth;
		const int mnTiles;
		const float mtileSize;
		Vertex::InstanceData* mVBData;
	};

}
#endif