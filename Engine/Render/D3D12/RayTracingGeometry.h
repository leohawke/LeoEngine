/*! \file Engine\Render\D3D12\RayTracingGeometry.h
\ingroup Engine
\brief 射线几何信息实现类。
*/
#ifndef LE_RENDER_D3D12_RayTracingGeometry_h
#define LE_RENDER_D3D12_RayTracingGeometry_h 1

#include "../IRayTracingGeometry.h"
#include "../IRayDevice.h"
#include "d3d12_dxgi.h"
#include "GraphicsBuffer.hpp"

namespace platform_ex::Windows::D3D12 {
	struct RayTracingGeometrySegement
	{
		GraphicsBuffer* VertexBuffer = nullptr;

		//todo:define direct type
		platform::Render::EFormat VertexFormat = EF_BGR32F;

		// Offset in bytes from the base address of the vertex buffer.
		leo::uint32 VertexBufferOffset = 0;

		// Number of bytes between elements of the vertex buffer (sizeof EF_BGR32F by default).
		// Must be equal or greater than the size of the position vector.
		leo::uint32 VertexBufferStride = 12;

		// Primitive range for this segment.
		leo::uint32 FirstPrimitive = 0;
		leo::uint32 NumPrimitives = 0;

		void operator=(const platform::Render::RayTracingGeometrySegement& segment)
		{
			VertexBuffer = static_cast<GraphicsBuffer*>(segment.VertexBuffer);
			VertexFormat = segment.VertexFormat;
			VertexBufferOffset = segment.VertexBufferOffset;
			VertexBufferStride = segment.VertexBufferStride;
			FirstPrimitive = segment.FirstPrimitive;
			NumPrimitives = segment.NumPrimitives;
		}
	};

	/** Bottom level ray tracing acceleration structure (contains triangles). */
	class RayTracingGeometry:public platform::Render::RayTracingGeometry
	{
	public:
		leo::uint32 IndexStride = 0; // 0 for non-indexed / implicit triangle list, 2 for uint16, 4 for uint32
		leo::uint32 IndexOffsetInBytes = 0;

		D3D12_RAYTRACING_GEOMETRY_TYPE GeometryType = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;

		RayTracingGeometrySegement Segment;
		D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS BuildFlags;


		GraphicsBuffer* IndexBuffer;

		void BuildAccelerationStructure();
	};
}

#endif