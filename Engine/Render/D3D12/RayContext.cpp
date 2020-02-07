#include "RayContext.h"
#include "RayDevice.h"
#include "Context.h"
#include "GraphicsBuffer.hpp"

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

void D12::RayDevice::BuildAccelerationStructure(R::RayTracingGeometry* geometry)
{
	RayTracingGeometry* pGeometry = static_cast<RayTracingGeometry*>(geometry);


	//TODO:transition all vbs and ibs to readable state

	pGeometry->BuildAccelerationStructure();
}

D12::RayContext::RayContext(Device* pDevice, Context* pContext)
	:device(pDevice),context(pContext)
{
	ray_device = std::make_shared<RayDevice>(pDevice,pContext);

	context->GetCommandList(Device::Command_Render)->QueryInterface(COMPtr_RefParam(raytracing_command_list, IID_ID3D12GraphicsCommandList4));

	if(!IsDirectXRaytracingSupported(pDevice->d3d_device.Get()))
		throw leo::unsupported();
}