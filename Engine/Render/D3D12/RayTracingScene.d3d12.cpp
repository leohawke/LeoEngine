#include "RayTracingScene.h"
#include "Context.h"
#include "RayTracingGeometry.h"

using namespace platform_ex::Windows::D3D12;
using namespace platform::Render::Buffer;

RayTracingScene::RayTracingScene(const platform::Render::RayTracingSceneInitializer& initializer)
	:Instances(initializer.Instances.begin(),initializer.Instances.end()),
	ShaderSlotsPerGeometrySegment(initializer.ShaderSlotsPerGeometrySegment),
	NumCallableShaderSlots(initializer.NumCallableShaderSlots)
{
	RayTracingDevice = Context::Instance().GetRayContext().GetDevice().GetRayTracingDevice();
}

void RayTracingScene::BuildAccelerationStructure()
{
	auto& device = Context::Instance().GetDevice();

	auto BuildFlags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;

	uint32 NumDxrInstances =static_cast<uint32>(Instances.size());

	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS PrebuildDescInputs = {};

	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO PrebuildInfo = {};
	PrebuildDescInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
	PrebuildDescInputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
	PrebuildDescInputs.NumDescs = NumDxrInstances;
	PrebuildDescInputs.Flags = BuildFlags;

	RayTracingDevice->GetRaytracingAccelerationStructurePrebuildInfo(&PrebuildDescInputs, &PrebuildInfo);

	shared_ptr<GraphicsBuffer> ScratchBuffer;
	CreateAccelerationStructureBuffers(AccelerationStructureBuffer, ScratchBuffer, device, PrebuildInfo);

	//scratch buffers should be created in UAV state from the start
	D3D12_RESOURCE_BARRIER barrier;
	barrier.Transition.Subresource = 0;
	ScratchBuffer->UpdateResourceBarrier(barrier, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);

	Context::Instance().GetCommandList(Device::Command_Resource)->ResourceBarrier(1, &barrier);

	AccelerationStructureView = leo::make_unique<ShaderResourceView>(*AccelerationStructureBuffer,EF_Unknown);

	// Create and fill instance buffer
	const uint32 NumSceneInstances =static_cast<uint32>(Instances.size());

	unique_ptr<GraphicsBuffer> InstanceBuffer;

	if (NumSceneInstances)
	{
		InstanceBuffer =leo::unique_raw(device.CreateVertexBuffer(Usage::Static, EAccessHint::EA_CPUWrite,
			sizeof(D3D12_RAYTRACING_INSTANCE_DESC) * PrebuildDescInputs.NumDescs,
			EFormat::EF_Unknown));

		Mapper mapper(*InstanceBuffer, Access::Write_Only);

		auto MappedData = mapper.Pointer< D3D12_RAYTRACING_INSTANCE_DESC>();

		LAssertNonnull(MappedData);

		for (uint32 InstanceIndex = 0; InstanceIndex < NumSceneInstances; ++InstanceIndex)
		{
			auto& Instance = Instances[InstanceIndex];

			auto Geometry = static_cast<RayTracingGeometry*>(Instance.Geometry);

			D3D12_RAYTRACING_INSTANCE_DESC InstanceDesc = {};

			InstanceDesc.InstanceMask = Instance.Mask;
			InstanceDesc.InstanceContributionToHitGroupIndex = InstanceIndex * ShaderSlotsPerGeometrySegment;

			InstanceDesc.Flags = D3D12_RAYTRACING_INSTANCE_FLAG_TRIANGLE_FRONT_COUNTERCLOCKWISE;

			bool bForceOpaque = false;
			if (bForceOpaque)
			{
				InstanceDesc.Flags |= D3D12_RAYTRACING_INSTANCE_FLAG_FORCE_OPAQUE;
			}

			bool bDoubleSided = false;
			if (bDoubleSided)
			{
				InstanceDesc.Flags |= D3D12_RAYTRACING_INSTANCE_FLAG_TRIANGLE_CULL_DISABLE;
			}

			InstanceDesc.AccelerationStructure = Geometry->AccelerationStructureBuffer->Resource()->GetGPUVirtualAddress();
			
			InstanceDesc.InstanceID = 0;

			auto TransformTransposed = leo::math::transpose(Instance.Transform);

			memcpy(InstanceDesc.Transform, TransformTransposed);

			MappedData[InstanceIndex] = InstanceDesc;
		}
	}

	const bool IsUpdateMode = false;
	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC BuildDesc = {};
	BuildDesc.Inputs = PrebuildDescInputs;
	BuildDesc.Inputs.InstanceDescs = InstanceBuffer->Resource()->GetGPUVirtualAddress();
	BuildDesc.DestAccelerationStructureData = AccelerationStructureBuffer->Resource()->GetGPUVirtualAddress();
	BuildDesc.ScratchAccelerationStructureData = ScratchBuffer->Resource()->GetGPUVirtualAddress();
	BuildDesc.SourceAccelerationStructureData = D3D12_GPU_VIRTUAL_ADDRESS(0);

	auto RayTracingCommandList = Context::Instance().GetRayContext().RayTracingCommandList();

	Context::Instance().ExecuteUAVBarrier();

	RayTracingCommandList->BuildRaytracingAccelerationStructure(&BuildDesc, 0, nullptr);

	Context::Instance().ExecuteUAVBarrier();
}
