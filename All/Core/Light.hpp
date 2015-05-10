//  Copyright (C), FNS Studios, 2014-2014.
// -------------------------------------------------------------------------
//  File name:   Core/Light.hpp
//  Version:     v1.00
//  Created:     05/09/2015 by leo hawke.
//  Compilers:   Visual Studio.NET 2013
//  Description: 提供光照结构体和相关函数
// -------------------------------------------------------------------------
//  History:
//				
//
////////////////////////////////////////////////////////////////////////////

#ifndef Core_Light_Hpp
#define Core_Light_Hpp

#include <leo2dmath.hpp>

namespace leo {
	struct PointLight;
	struct DirectionalLight;
	struct SpotLight;

	struct PointLight {
		float4 PositionRange;
		float3 Diffuse;
	};

	struct DirectionalLight {
		float3 Directional;
		float3 Diffuse;
	};

	struct SpotLight : public PointLight {
		float4 DirectionalRadius;
	};

	class Camera;

	//windows_system
	ops::Rect CalcScissorRect(const PointLight& wPointLight, const Camera& camera);
}


#endif