////////////////////////////////////////////////////////////////////////////
//
//  Leo Engine Source File.
//  Copyright (C), FNS Studios, 2014-2014.
// -------------------------------------------------------------------------
//  File name:   Core/Material.h
//  Version:     v1.00
//  Created:     11/14/2014 by leo hawke.
//  Compilers:   Visual Studio.NET 2013
//  Description: 提供Material相关数据结构
// -------------------------------------------------------------------------
//  History:
//				
//
////////////////////////////////////////////////////////////////////////////

#ifndef Core_Material_H
#define Core_Material_H

#include "..\IndePlatform\LeoMath.h"

namespace leo{
	struct Material
	{
		float4 ambient;
		float4 diffuse;
		float4 specular; // w = SpecPower
	};
}

#endif