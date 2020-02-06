#include "RayTracingGeometry.h"
#include "Context.h"
#include "Convert.h"

namespace D12 = platform_ex::Windows::D3D12;
namespace R = platform::Render;

using namespace D12;

static void CreateAccelerationStructureBuffers(shared_ptr<GraphicsBuffer>& AccelerationStructureBuffer, shared_ptr<GraphicsBuffer>& ScratchBuffer, Device& Deivce, const D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO& PrebuildInfo);

void D12::RayTracingGeometry::BuildAccelerationStructure()
{
	auto& raydevice = Context::Instance().GetRayContext().GetDevice();

	lconstexpr leo::uint32 IndicesPerPrimitive = 3; // Only triangle meshes are supported

	D3D12_RAYTRACING_GEOMETRY_DESC Desc;

	Desc.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_NONE;
	Desc.Type = GeometryType;

	bool allowAnyHitShader = false;
	bool allowDuplicateAnyHitShaderInvocation = true;

	if (!allowAnyHitShader)
	{
		Desc.Flags |= D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;
	}

	if (!allowDuplicateAnyHitShaderInvocation)
	{
		Desc.Flags |= D3D12_RAYTRACING_GEOMETRY_FLAG_NO_DUPLICATE_ANYHIT_INVOCATION;
	}

	if (GeometryType == D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES)
	{
		Desc.Triangles.VertexFormat = Convert(Segement.VertexFormat);

		Desc.Triangles.Transform3x4 = D3D12_GPU_VIRTUAL_ADDRESS(0);

		if (IndexBuffer)
		{
			Desc.Triangles.IndexFormat = IndexStride == 4 ? DXGI_FORMAT_R32_UINT : DXGI_FORMAT_R16_UINT;
			Desc.Triangles.IndexCount = Segement.NumPrimitives * IndicesPerPrimitive;
			Desc.Triangles.IndexBuffer = IndexBuffer->Resource()->GetGPUVirtualAddress() +
				IndexOffsetInBytes +
				IndexStride * Segement.FirstPrimitive * IndicesPerPrimitive;

			Desc.Triangles.VertexCount = Segement.VertexBuffer->GetSize() / Segement.VertexBufferStride;
		}
		else // Non-indexed geometry
		{
			Desc.Triangles.IndexFormat = DXGI_FORMAT_UNKNOWN;
			Desc.Triangles.IndexCount = 0;
			Desc.Triangles.IndexBuffer = D3D12_GPU_VIRTUAL_ADDRESS(0);

			Desc.Triangles.VertexCount = Segement.VertexBuffer->GetSize() / Segement.VertexBufferStride;
		}

		Desc.Triangles.VertexBuffer.StartAddress = Segement.VertexBuffer->Resource()->GetGPUVirtualAddress() + Segement.VertexBufferOffset;
		Desc.Triangles.VertexBuffer.StrideInBytes = Segement.VertexBufferStride;
	}
	else if (GeometryType == D3D12_RAYTRACING_GEOMETRY_TYPE_PROCEDURAL_PRIMITIVE_AABBS)
	{
		//TODO
	}

	bool isUpdate = false;

	auto RayTracingDevice = raydevice.GetRayTracingDevice();
	
	auto LocalBuildFlags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS(BuildFlags);

	if (isUpdate)
	{
		LocalBuildFlags |= D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PERFORM_UPDATE;
	}

	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS PrebuildDescInputs = {};

	PrebuildDescInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
	PrebuildDescInputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
	PrebuildDescInputs.NumDescs =1;
	PrebuildDescInputs.pGeometryDescs = &Desc;
	PrebuildDescInputs.Flags = LocalBuildFlags;

	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO PrebuildInfo = {};
	RayTracingDevice->GetRaytracingAccelerationStructurePrebuildInfo(&PrebuildDescInputs, &PrebuildInfo);

	CreateAccelerationStructureBuffers(AccelerationStructureBuffer,ScratchBuffer, Context::Instance().GetDevice(), PrebuildInfo);

	//ScratchBuffer->UpdateResourceBarrier()

	Context::Instance().CommitCommandList(Device::Command_Resource);

	// We don't need to keep a scratch buffer after initial build if acceleration structure is static.
	if (!(BuildFlags & D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_ALLOW_UPDATE))
	{
		ScratchBuffer.reset();
	}
}

using namespace R::Buffer;
namespace D3D = platform_ex::Windows::D3D;

static void CreateAccelerationStructureBuffers(shared_ptr<GraphicsBuffer>& AccelerationStructureBuffer, shared_ptr<GraphicsBuffer>& ScratchBuffer, Device& Creator, const D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO& PrebuildInfo)
{
	AccelerationStructureBuffer =leo::share_raw(Creator.CreateVertexBuffer(
		Usage::AccelerationStructure,
		EAccessHint::EA_GPUUnordered,
		PrebuildInfo.ResultDataMaxSizeInBytes,
		EF_Unknown
	));

	D3D::Debug(AccelerationStructureBuffer->Resource(), "Acceleration structure");

	auto ScratchBufferWidth = std::max(PrebuildInfo.UpdateScratchDataSizeInBytes, PrebuildInfo.ScratchDataSizeInBytes);

	ScratchBuffer = leo::share_raw(Creator.CreateVertexBuffer(
		Usage::Static,
		EAccessHint::EA_GPUUnordered,
		ScratchBufferWidth,
		EF_Unknown
	));

	D3D::Debug(AccelerationStructureBuffer->Resource(), "Acceleration structure scratch");
}
