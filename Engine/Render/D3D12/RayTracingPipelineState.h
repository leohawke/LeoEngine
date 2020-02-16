#pragma once
#ifndef LE_RENDER_D3D12_RayTracingPipelineState_h
#define LE_RENDER_D3D12_RayTracingPipelineState_h 1

#include "../IRayTracingPipelineState.h"
#include "../IRayDevice.h"
#include "d3d12_dxgi.h"
#include "D3D12RayTracing.h"
#include "RootSignature.h"

namespace platform_ex::Windows::D3D12 {
	class RayTracingPipelineState : public platform::Render::RayTracingPipelineState
	{
	public:
		RayTracingPipelineState(const platform::Render::RayTracingPipelineStateInitializer& initializer);
	public:
		COMPtr<ID3D12StateObject> StateObject;

		// This is useful for the case where user only provides default RayGen, Miss and HitGroup shaders.
		::RayTracingShaderTable DefaultShaderTable;

		::RayTracingShaderLibrary RayGenShaders;
		::RayTracingShaderLibrary MissShaders;
		::RayTracingShaderLibrary HitGroupShaders;

		leo::observer_ptr<RootSignature> GlobalRootSignature = nullptr;

		bool bAllowHitGroupIndexing = true;
		leo::uint32 MaxLocalRootSignatureSize = 0;
		leo::uint32 MaxHitGroupViewDescriptors = 0;
	};
}

#endif