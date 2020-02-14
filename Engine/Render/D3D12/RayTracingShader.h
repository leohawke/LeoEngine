#pragma once
#ifndef LE_RENDER_D3D12_RayTracingShader_h
#define LE_RENDER_D3D12_RayTracingShader_h 1

#include "../IRayTracingShader.h"
#include "d3d12_dxgi.h"

namespace platform_ex::Windows::D3D12 {
	class RayTracingShader : public platform::Render::RayTracingShader
	{
	public:
		ID3D12RootSignature* pRootSigneature = nullptr;
	};
}

#endif