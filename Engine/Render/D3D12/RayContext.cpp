#include "RayContext.h"
#include "RayDevice.h"

namespace D12 = platform_ex::Windows::D3D12;
namespace R = platform::Render;

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

	throw leo::unimplemented();
}