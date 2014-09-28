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

namespace leo
{
	template<size_t MAXLOD = 4,size_t MAXEDGEVERTEX = 256,size_t TRIWIDTH = 12>
	//static_assert(pow(2,MAXLOD) <= MAXEDGEVERTEX)
	class Terrain
	{
	public:
		Terrain()
		{}
#ifdef LB_IMPL_MSCPP
		static_assert(leo::constexprmath::pow<2, MAXLOD>::value <= MAXEDGEVERTEX, "The n LOD's edgevertex is 2 times (n+1) LOD's edgevertex");
#else
		static_assert(leo::constexprmath::pow<2, MAXLOD>() <= MAXEDGEVERTEX, "The n LOD's edgevertex is 2 times (n+1) LOD's edgevertex");
#endif
		class Chunk
		{

		};
		
	};
}

#endif