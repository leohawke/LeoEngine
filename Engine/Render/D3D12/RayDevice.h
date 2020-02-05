#ifndef LE_RENDER_D3D12_RayDevice_h
#define LE_RENDER_D3D12_RayDevice_h 1

#include "../IRayDevice.h"
#include "d3d12_dxgi.h"
#include "RayTracingGeometry.h"

namespace platform_ex::Windows::D3D12 {
	class Device;
	class Context;

	class RayDevice:public platform::Render::RayDevice
	{
	public:
		RayDevice(Device* pDevice, Context* pContext);

		RayTracingGeometry* CreateRayTracingGeometry(const platform::Render::RayTracingGeometryInitializer& initializer) final override;

		//TODO:state abstract cause performance(GPU sync point)
		void BuildAccelerationStructure(platform::Render::RayTracingGeometry* pGeometry) final override;
	private:
		Device* device;
		Context* context;
	};

	bool IsDirectXRaytracingSupported(ID3D12Device* device);
}

#endif
