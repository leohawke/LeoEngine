/*! \file Engine\Render\D3D12\RayTracingGeometry.h
\ingroup Engine
\brief 射线几何信息实现类。
*/
#ifndef LE_RENDER_D3D12_RayTracingGeometry_h
#define LE_RENDER_D3D12_RayTracingGeometry_h 1

#include "../IRayTracingGeometry.h"

namespace platform_ex::Windows::D3D12 {
	/** Bottom level ray tracing acceleration structure (contains triangles). */
	class RayTracingGeometry:public platform::Render::RayTracingGeometry
	{
	};
}

#endif