/*! \file Engine\Render\D3D12\RayTracingGeometry.h
\ingroup Engine
\brief 射线几何信息实现类。
*/
#ifndef LE_RENDER_D3D12_RayTracingGeometry_h
#define LE_RENDER_D3D12_RayTracingGeometry_h 1

#include "../IRayTracingGeometry.h"
#include "../IRayDevice.h"
#include "d3d12_dxgi.h"

namespace platform_ex::Windows::D3D12 {
	/** Bottom level ray tracing acceleration structure (contains triangles). */
	class RayTracingGeometry:public platform::Render::RayTracingGeometry
	{
	public:
		leo::uint32 IndexStride = 0; // 0 for non-indexed / implicit triangle list, 2 for uint16, 4 for uint32
		leo::uint32 IndexOffsetInBytes = 0;

		D3D12_RAYTRACING_GEOMETRY_TYPE GeometryType = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;

		platform::Render::RayTracingGeometrySegement Segment;
		D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS BuildFlags;


		GraphicsBuffer* IndexBuffer;

		void BuildAccelerationStructure();
	};
}

#endif