#ifndef LE_RENDER_D3D12_RayDevice_h
#define LE_RENDER_D3D12_RayDevice_h 1

#include "../IRayDevice.h"

namespace platform_ex::Windows::D3D12 {
	class Device;
	class Context;

	class RayDevice:public platform::Render::RayDevice
	{
	public:
		RayDevice(Device* pDevice, Context* pContext);

	private:
		Device* device;
		Context* context;
	};
}

#endif
