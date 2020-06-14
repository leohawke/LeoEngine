/*! \file Engine\Render\D3D12\RayTracingScene.h
\ingroup Engine
\brief 射线场景信息实现类。
*/
#pragma once

#ifndef LE_RENDER_D3D12_RayTracingScene_h
#define LE_RENDER_D3D12_RayTracingScene_h 1

#include "../IRayTracingScene.h"
#include "d3d12_dxgi.h"
#include "../IRayDevice.h"
#include "GraphicsBuffer.hpp"
#include "RayTracingGeometry.h"
#include "View.h"

namespace platform_ex::Windows::D3D12 {
	class RayTracingScene :public platform::Render::RayTracingScene
	{
	public:
		RayTracingScene(const platform::Render::RayTracingSceneInitializer& initializer);

		void BuildAccelerationStructure();

		ShaderResourceView* GetShaderResourceView() const
		{
			return AccelerationStructureView.get();
		}
	private:
		std::vector<platform::Render::RayTracingGeometryInstance> Instances;

		leo::uint32 ShaderSlotsPerGeometrySegment;
		leo::uint32 NumCallableShaderSlots;

		std::shared_ptr<GraphicsBuffer> AccelerationStructureBuffer;

		ID3D12Device5* RayTracingDevice;

		leo::observer_ptr<ShaderResourceView> AccelerationStructureView;
	};
}

#endif
