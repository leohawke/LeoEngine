#include "RayContext.h"
#include "RayDevice.h"
#include "Context.h"
#include "GraphicsBuffer.hpp"
#include "ResourceView.h"

namespace D12 = platform_ex::Windows::D3D12;
namespace R = platform::Render;

using namespace D12;

bool D12::IsDirectXRaytracingSupported(ID3D12Device* device)
{
	D3D12_FEATURE_DATA_D3D12_OPTIONS5 featureSupportData = {};

	return SUCCEEDED(device->CheckFeatureSupport(D3D12_FEATURE_D3D12_OPTIONS5,&featureSupportData,sizeof(featureSupportData)))
		&& featureSupportData.RaytracingTier != D3D12_RAYTRACING_TIER_NOT_SUPPORTED;
}

D12::RayDevice& D12::RayContext::GetDevice()
{
	return *ray_device;
}

ID3D12GraphicsCommandList4* D12::RayContext::RayTracingCommandList() const
{
	return raytracing_command_list.Get();
}

D12::RayDevice::RayDevice(Device* pDevice, Context* pContext)
	:device(pDevice), context(pContext)
{
	if (!SUCCEEDED(device->d3d_device->QueryInterface(COMPtr_RefParam(d3d_ray_device, IID_ID3D12Device5))))
		throw leo::unsupported();

	ray_tracing_descriptor_heap_cache = make_unique<RayTracingDescriptorHeapCache>(this);
}

D12::RayTracingGeometry* D12::RayDevice::CreateRayTracingGeometry(const R::RayTracingGeometryInitializer& initializer)
{
	D12::RayTracingGeometry* Geometry = new D12::RayTracingGeometry(initializer);

	return Geometry;
}

RayTracingScene* D12::RayDevice::CreateRayTracingScene(const R::RayTracingSceneInitializer& initializer)
{
	auto Scene = new RayTracingScene(initializer);

	return Scene;
}

RayTracingPipelineState* D12::RayDevice::CreateRayTracingPipelineState(const platform::Render::RayTracingPipelineStateInitializer& initializer)
{
	auto PipelineState = new RayTracingPipelineState(initializer);

	return PipelineState;
}

RayTracingShader* D12::RayDevice::CreateRayTracingSahder(const platform::Render::RayTracingShaderInitializer& initializer)
{
	auto Shader = new RayTracingShader(initializer);

	return Shader;
}

void D12::RayDevice::BuildAccelerationStructure(R::RayTracingGeometry* geometry)
{
	RayTracingGeometry* pGeometry = static_cast<RayTracingGeometry*>(geometry);

	pGeometry->BuildAccelerationStructure();
}

void D12::RayDevice::BuildAccelerationStructure(R::RayTracingScene* scene)
{
	RayTracingScene* pScene = static_cast<RayTracingScene*>(scene);


	pScene->BuildAccelerationStructure();
}

const D12::Fence& platform_ex::Windows::D3D12::RayDevice::GetFence() const
{
	return device->GetRenderFence();
}

RayTracingDescriptorHeapCache* platform_ex::Windows::D3D12::RayDevice::GetRayTracingDescriptorHeapCache() const
{
	return ray_tracing_descriptor_heap_cache.get();
}

D12::RayContext::RayContext(Device* pDevice, Context* pContext)
	:device(pDevice),context(pContext)
{
	ray_device = std::make_shared<RayDevice>(pDevice,pContext);

	context->GetCommandList(Device::Command_Render)->QueryInterface(COMPtr_RefParam(raytracing_command_list, IID_ID3D12GraphicsCommandList4));

	if(!IsDirectXRaytracingSupported(pDevice->d3d_device.Get()))
		throw leo::unsupported();
}

void D12::RayContext::RayTraceShadow(R::RayTracingScene* InScene, R::FrameBuffer* InDepth,R::UnorderedAccessView* Output, R::GraphicsBuffer* InConstants)
{
	auto Depth = static_cast<D12::FrameBuffer*>(InDepth);
	auto DepthView = Depth->GetDepthStencilView();

	auto Resource = DepthView->GetResourceHolder();

	auto ITex = dynamic_cast<R::Texture*>(Resource);
	auto Tex = dynamic_cast<D12::Texture*>(Resource);

	D12::ShaderResourceView DepthSRV{ Tex, ITex,DepthView->Width(),DepthView->Height()};

	D3D12_RESOURCE_BARRIER barrier;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	if (Resource->UpdateResourceBarrier(barrier, D3D12_RESOURCE_STATE_DEPTH_READ))
	{
		raytracing_command_list->ResourceBarrier(1,&barrier);
	}

	RayTracingShaderBindings Bindings;

	Bindings.SRVs[0] =static_cast<D12::RayTracingScene*>(InScene)->GetShaderResourceView();
	Bindings.SRVs[1] = &DepthSRV;
	Bindings.UniformBuffers[0] = InConstants;
	Bindings.UAVs[0] = Output;

	D12::RayTracingPipelineState* Pipeline = nullptr;//GetBasicRayTracingPipeline()->Shadow;

	auto& ShaderTable = Pipeline->DefaultShaderTable;
	ShaderTable.UploadToGPU(&Context::Instance().GetDevice());

	D3D12_DISPATCH_RAYS_DESC DispatchDesc = ShaderTable.GetDispatchRaysDesc(0, 0, 0);
	DispatchDesc.Width = DepthView->Width();
	DispatchDesc.Height = DepthView->Height();
	DispatchDesc.Depth = 1;

	DispatchRays(this, Bindings, Pipeline, 0, nullptr, DispatchDesc);
}