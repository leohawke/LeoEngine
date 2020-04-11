#pragma once
#ifndef LE_RENDER_D3D12_RayTracingShader_h
#define LE_RENDER_D3D12_RayTracingShader_h 1

#include <LBase/observer_ptr.hpp>
#include "../ShaderCore.h"
#include "../IRayTracingShader.h"
#include "../IRayDevice.h"
#include "d3d12_dxgi.h"
#include "RootSignature.h"

namespace platform_ex::Windows::D3D12 {
	class RayTracingShader : public platform::Render::RayTracingShader
	{
	public:
		RayTracingShader(const  platform::Render::RayTracingShaderInitializer& initializer);
	public:
		leo::observer_ptr<RootSignature> pRootSignature = nullptr;

		platform::Render::ShaderBlob ShaderByteCode;

		std::u16string EntryPoint;
		std::u16string AnyHitEntryPoint;
		std::u16string IntersectionEntryPoint;

		platform::Render::ShaderCodeResourceCounts ResourceCounts;
	};

	bool IsRayTracingShader(platform::Render::ShaderType type);
}

#endif