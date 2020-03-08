#ifndef LE_RENDER_D3D12_RayDevice_h
#define LE_RENDER_D3D12_RayDevice_h 1

#include "../IRayDevice.h"
#include "d3d12_dxgi.h"
#include "RayTracingGeometry.h"
#include "RayTracingScene.h"
#include "RayTracingPipelineState.h"
#include "RayTracingShader.h"
#include "Fence.h"

class RayTracingDescriptorHeapCache;

namespace platform_ex::Windows::D3D12 {
	class Device;
	class Context;

	class RayDevice:public platform::Render::RayDevice
	{
	public:
		RayDevice(Device* pDevice, Context* pContext);

		RayTracingGeometry* CreateRayTracingGeometry(const platform::Render::RayTracingGeometryInitializer& initializer) final override;

		RayTracingScene* CreateRayTracingScene(const platform::Render::RayTracingSceneInitializer& initializer)  final override;

		RayTracingPipelineState* CreateRayTracingPipelineState(const platform::Render::RayTracingPipelineStateInitializer& initializer) final override;

		RayTracingShader* CreateRayTracingSahder(const  platform::Render::RayTracingShaderInitializer& initializer) final override;


		//TODO:state abstract cause performance(GPU sync point)
		void BuildAccelerationStructure(platform::Render::RayTracingGeometry* pGeometry) final override;
		void BuildAccelerationStructure(platform::Render::RayTracingScene* pGeometry) final override;

		ID3D12Device5* GetRayTracingDevice() const
		{
			return d3d_ray_device.Get();
		}

		const Fence& GetFence() const;

		RayTracingDescriptorHeapCache* GetRayTracingDescriptorHeapCache() const;
	private:
		Device* device;
		Context* context;

		COMPtr<ID3D12Device5> d3d_ray_device;
		unique_ptr< RayTracingDescriptorHeapCache> ray_tracing_descriptor_heap_cache;
	};

	bool IsDirectXRaytracingSupported(ID3D12Device* device);
}

#endif
