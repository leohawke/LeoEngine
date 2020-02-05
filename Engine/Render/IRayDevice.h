/*! \file Engine\Render\IRayDevice.h
\ingroup Engine
\brief 射线创建接口类。
*/
#ifndef LE_RENDER_IRayDevice_h
#define LE_RENDER_IRayDevice_h 1

#include "IGraphicsBuffer.hpp"
#include "IFormat.hpp"

namespace platform::Render {

	class RayTracingGeometry;

	enum class ERayTracingGeometryType
	{
		// Indexed or non-indexed triangle list with fixed function ray intersection.
		// Vertex buffer must contain vertex positions as float3.
		// Vertex stride must be at least 12 bytes, but may be larger to support custom per-vertex data.
		// Index buffer may be provided for indexed triangle lists. Implicit triangle list is assumed otherwise.
		Triangles,

		// Custom primitive type that requires an intersection shader.
		// Vertex buffer for procedural geometry must contain one AABB per primitive as {float3 MinXYZ, float3 MaxXYZ}.
		// Vertex stride must be at least 24 bytes, but may be larger to support custom per-primitive data.
		// Index buffers can't be used with procedural geometry.
		Procedural,
	};

	struct RayTracingGeometrySegement
	{
		GraphicsBuffer* VertexBuffer = nullptr;

		//todo:define direct type
		EFormat VertexFormat = EF_BGR32F;

		// Offset in bytes from the base address of the vertex buffer.
		leo::uint32 VertexBufferOffset = 0;

		// Number of bytes between elements of the vertex buffer (sizeof EF_BGR32F by default).
		// Must be equal or greater than the size of the position vector.
		leo::uint32 VertexBufferStride = 12;

		// Primitive range for this segment.
		leo::uint32 FirstPrimitive = 0;
		leo::uint32 NumPrimitives = 0;
	};

	struct RayTracingGeometryInitializer
	{
		GraphicsBuffer* IndexBuffer = nullptr;

		// Offset in bytes from the base address of the index buffer.
		leo::uint32 IndexBufferOffset = 0;

		ERayTracingGeometryType GeometryType = ERayTracingGeometryType::Triangles;

		RayTracingGeometrySegement Segement;
	};

	class RayDevice
	{
	public:
		virtual RayTracingGeometry* CreateRayTracingGeometry(const RayTracingGeometryInitializer& initializer) = 0;

		virtual void BuildAccelerationStructure(platform::Render::RayTracingGeometry* pGeometry) =0;
	};
}

#endif