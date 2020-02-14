#pragma once
#ifndef LE_RENDER_D3D12_RayTracingPipelineState_h
#define LE_RENDER_D3D12_RayTracingPipelineState_h 1

#include "../IRayTracingPipelineState.h"
#include "../IRayDevice.h"
#include "d3d12_dxgi.h"

namespace platform_ex::Windows::D3D12 {
	class RayTracingPipelineState : public platform::Render::RayTracingPipelineState
	{
	public:
		RayTracingPipelineState(const platform::Render::RayTracingPipelineStateInitializer& initializer);
	public:
		COMPtr<ID3D12StateObject> StateObject;
	};
}

#endif