////////////////////////////////////////////////////////////////////////////
//
//  Leo Engine Source File.
//  Copyright (C), FNS Studios, 2014-2014.
// -------------------------------------------------------------------------
//  File name:   Core/Lod.h
//  Version:     v1.00
//  Created:     11/14/2014 by leo hawke.
//  Compilers:   Visual Studio.NET 2013
//  Description: 提供LOD相关数据结构
// -------------------------------------------------------------------------
//  History:
//				
//
////////////////////////////////////////////////////////////////////////////

#ifndef Core_Lod_H
#define Core_Lod_H

#include <cstdint>

namespace leo{
	struct LodIndex{
		std::uint32_t mOffset;
		std::uint32_t mCount;
	};
}

#endif