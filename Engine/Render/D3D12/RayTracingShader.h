#pragma once
#ifndef LE_RENDER_D3D12_RayTracingShader_h
#define LE_RENDER_D3D12_RayTracingShader_h 1

#include <LBase/lmemory.hpp>
#include "../ShaderCore.h"
#include "../IRayTracingShader.h"
#include "d3d12_dxgi.h"
#include "RootSignature.h"

namespace platform_ex::Windows::D3D12 {
	class RayTracingShader : public platform::Render::RayTracingShader
	{
	public:
		leo::observer_ptr<RootSignature> pRootSignature = nullptr;

		platform::Render::ShaderCodeResourceCounts ResourceCounts;
	};
}

#endif