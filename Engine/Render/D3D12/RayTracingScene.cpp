#include "RayTracingScene.h"
#include "Context.h"
#include "RayTracingGeometry.h"

using namespace platform_ex::Windows::D3D12;

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

	uint32 NumDxrInstances = Instances.size();

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

	{
		D3D12_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
		SRVDesc.Format = DXGI_FORMAT_UNKNOWN;
		SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_RAYTRACING_ACCELERATION_STRUCTURE;
		SRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		SRVDesc.RaytracingAccelerationStructure.Location = AccelerationStructureBuffer->Resource()->GetGPUVirtualAddress();

		AccelerationStructureView = leo::make_unique< ViewSimulation>(COMPtr<ID3D12Resource>(AccelerationStructureBuffer->Resource()), SRVDesc);
	}

	// Create and fill instance buffer
}
