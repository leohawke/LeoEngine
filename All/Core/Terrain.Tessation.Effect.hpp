////////////////////////////////////////////////////////////////////////////
//
//  Leo Engine Source File.
//  Copyright (C), FNS Studios, 2014-2014.
// -------------------------------------------------------------------------
//  File name:   Core/Terrain.Tessation.Effect.hpp
//  Version:     v1.00
//  Created:     9/5/2014 by leo hawke.
//  Compilers:   Visual Studio.NET 2013
//  Description: 提供地形曲面细分的效果接口
// -------------------------------------------------------------------------
//  History:
//				
//
////////////////////////////////////////////////////////////////////////////
#ifndef Core_Terrain_Tessation_Effect_hpp
#define Core_Terrain_Tessation_Effect_hpp

#include "effect.h"
#include "..\leomath.hpp"
#include "..\IndePlatform\leoint.hpp"
#include "..\IndePlatform\utility.hpp"
struct D3D11_INPUT_ELEMENT_DESC;
namespace leo
{
	namespace Vertex
	{
		struct Adjacency
		{
			// These are the size of the neighbours along +/- x or y axes.  For interior tiles
			// this is 1.  For edge tiles it is 0.5 or 2.0.
			float neighbourMinusX;
			float neighbourMinusY;
			float neighbourPlusX;
			float neighbourPlusY;
		};
		struct Terrain
		{
			float2 position;
			Adjacency adjacency;
			//int32 VertexId;
			//int32 InstanceId;
		};
	}

	namespace InputLayoutDesc
	{
		extern const D3D11_INPUT_ELEMENT_DESC Terrain[5];
	}
}

namespace leo
{
	class TerrainTessationEffect :public Effect,ABSTRACT
	{
	public:
		void Apply(ID3D11DeviceContext* context);
		bool SetLevel(EffectConfig::EffectLevel l) lnothrow;
	public:
		static const std::unique_ptr<TerrainTessationEffect>& GetInstance(ID3D11Device* device = nullptr);
	};
}

#endif