#ifndef LE_RENDER_D3D12_RayContxt_h
#define LE_RENDER_D3D12_RayContxt_h 1

#include <LBase/lmemory.hpp>
#include "../IRayContext.h"
#include "RayDevice.h"

namespace platform_ex::Windows::D3D12 {
	class Device;
	class Context;

	class RayContext :public platform::Render::RayContext
	{
	public:
		RayContext(Device* pDevice, Context* pContext);

		RayDevice& GetDevice() override;
	private:
		std::shared_ptr<RayDevice> ray_device;

		Device* device;
		Context* context;
	};
}

#endif
