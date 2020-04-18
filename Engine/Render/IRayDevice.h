/*! \file Engine\Render\IRayDevice.h
\ingroup Engine
\brief 射线追踪资源创建类。
*/
#ifndef LE_RENDER_IRayDevice_h
#define LE_RENDER_IRayDevice_h 1

#include "IGraphicsBuffer.hpp"
#include "IFormat.hpp"
#include "../Asset/EffectAsset.h"
#include <LBase/span.hpp>
#include <LBase/lmathtype.hpp>
#include "IRayTracingShader.h"

namespace platform::Render {

	class RayTracingGeometry;
	class RayTracingScene;
	class RayTracingPipelineState;

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

	namespace math = leo::math;

	struct RayTracingGeometryInstance
	{
		RayTracingGeometry* Geometry;

		math::float4x4 Transform;

		leo::uint8 Mask = 0XFF;
	};

	// Scene may only be used during the frame when it was created.
	struct RayTracingSceneInitializer
	{
		leo::span< RayTracingGeometryInstance> Instances;

		// This value controls how many elements will be allocated in the shader binding table per geometry segment.
		// Changing this value allows different hit shaders to be used for different effects.
		// For example, setting this to 2 allows one hit shader for regular material evaluation and a different one for shadows.
		// Desired hit shader can be selected by providing appropriate RayContributionToHitGroupIndex to TraceRay() function.
		// Use ShaderSlot argument in SetRayTracingHitGroup() to assign shaders and resources for specific part of the shder binding table record.
		leo::uint32 ShaderSlotsPerGeometrySegment = 1;


		// Defines how many different callable shaders with unique resource bindings can be bound to this scene.
		// Shaders and resources are assigned to slots in the scene using SetRayTracingCallableShader().
		leo::uint32 NumCallableShaderSlots = 0;
	};

	class RayTracingPipelineStateInitializer
	{
	public:
		RayTracingPipelineStateInitializer() {};

		uint32 MaxPayloadSizeInBytes = 24;

		bool bAllowHitGroupIndexing = true;

	public:
		leo::span<RayTracingShader*> RayGenTable;
		leo::span<RayTracingShader*> MissTable;
		leo::span<RayTracingShader*> HitGroupTable;
		leo::span<RayTracingShader*> CallableTable;
	};

	class RayTracingShaderInitializer
	{
	public:
		const platform::Render::ShaderBlob* pBlob;
		const platform::Render::ShaderInfo* pInfo;
	};

	class RayDevice
	{
	public:
		virtual RayTracingGeometry* CreateRayTracingGeometry(const RayTracingGeometryInitializer& initializer) = 0;

		virtual RayTracingScene* CreateRayTracingScene(const RayTracingSceneInitializer& initializer) = 0;

		virtual RayTracingPipelineState* CreateRayTracingPipelineState(const RayTracingPipelineStateInitializer& initializer) = 0;

		virtual RayTracingShader* CreateRayTracingSahder(const RayTracingShaderInitializer& initializer) = 0;

		virtual void BuildAccelerationStructure(RayTracingGeometry* pGeometry) = 0;
		virtual void BuildAccelerationStructure(RayTracingScene* pGeometry) =0;
	};
}

#endif