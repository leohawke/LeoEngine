//  Copyright (C), FNS Studios, 2014-2015.
// -------------------------------------------------------------------------
//  File name:   Core/LightStruct.hpp
//  Version:     v1.00
//  Created:     07/06/2015 by leo hawke.
//  Compilers:   Visual Studio.NET 2015 RC
//  Description: 提供光照结构体
// -------------------------------------------------------------------------
//  History:
//				
//
////////////////////////////////////////////////////////////////////////////

#ifndef Core_LightStruct_Hpp
#define Core_LightStruct_Hpp

#include <leomathtype.hpp>

namespace leo {
	struct PointLight;
	struct DirectionalLight;
	struct SpotLight;

	struct PointLight {
		float3 Position;
		float3 Diffuse;
		float4 FallOff_Range;
	};

	struct DirectionalLight {
		float3 Directional;
		float3 Diffuse;
	};

	struct SpotLight {
		float4 Position_Inner;
		float3 Diffuse;
		float4 FallOff_Range;
		float4 Directional_Outer;
	};

}

#endif
