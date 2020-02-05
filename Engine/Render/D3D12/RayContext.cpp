#include "RayContext.h"
#include "RayDevice.h"
#include "Context.h"

namespace D12 = platform_ex::Windows::D3D12;
namespace R = platform::Render;


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

D12::RayDevice::RayDevice(Device* pDevice, Context* pContext)
	:device(pDevice), context(pContext)
{
}

D12::RayContext::RayContext(Device* pDevice, Context* pContext)
	:device(pDevice),context(pContext)
{
	ray_device = std::make_shared<RayDevice>(pDevice,pContext);

	if(IsDirectXRaytracingSupported(pDevice->d3d_device.Get()))
		throw leo::unsupported();
}