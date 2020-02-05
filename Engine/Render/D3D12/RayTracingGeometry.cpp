#include "RayTracingGeometry.h"
#include "Context.h"
#include "Convert.h"

namespace D12 = platform_ex::Windows::D3D12;
namespace R = platform::Render;

using namespace D12;

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
		Desc.Triangles.VertexFormat = Convert(Segment.VertexFormat);

		Desc.Triangles.Transform3x4 = D3D12_GPU_VIRTUAL_ADDRESS(0);

		if (IndexBuffer)
		{
			Desc.Triangles.IndexFormat = IndexStride == 4 ? DXGI_FORMAT_R32_UINT : DXGI_FORMAT_R16_UINT;
			Desc.Triangles.IndexCount = Segment.NumPrimitives * IndicesPerPrimitive;
			Desc.Triangles.IndexBuffer = IndexBuffer->Resource()->GetGPUVirtualAddress() +
				IndexOffsetInBytes +
				IndexStride * Segment.FirstPrimitive * IndicesPerPrimitive;

			Desc.Triangles.VertexCount = Segment.VertexBuffer->GetSize() / Segment.VertexBufferStride;
		}
		else // Non-indexed geometry
		{
			Desc.Triangles.IndexFormat = DXGI_FORMAT_UNKNOWN;
			Desc.Triangles.IndexCount = 0;
			Desc.Triangles.IndexBuffer = D3D12_GPU_VIRTUAL_ADDRESS(0);

			Desc.Triangles.VertexCount = Segment.VertexBuffer->GetSize() / Segment.VertexBufferStride;
		}

		Desc.Triangles.VertexBuffer.StartAddress = Segment.VertexBuffer->Resource()->GetGPUVirtualAddress() + Segment.VertexBufferOffset;
		Desc.Triangles.VertexBuffer.StrideInBytes = Segment.VertexBufferStride;
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

	//CreateAccelerationStructureBuffers

	Context::Instance().CommitCommandList(Device::Command_Resource);
}