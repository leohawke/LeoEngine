#pragma once

#ifndef LE_RENDER_D3D12_RootSignature_h
#define LE_RENDER_D3D12_RootSignature_h 1

#include "../ShaderCore.h"
#include "d3d12_dxgi.h"
#include <LBase/lmemory.hpp>
#include <mutex>

namespace platform_ex::Windows::D3D12 {
	using platform::Render::ShaderBlob;
	using platform::Render::ShaderCodeResourceCounts;

	constexpr leo::uint32 ShaderVisibilityCount = 6;

	enum class RootSignatureType
	{
		Raster,
		RayTracingGlobal,
		RayTracingLocal
	};


	struct QuantizedBoundShaderState
	{
		ShaderCodeResourceCounts RegisterCounts[ShaderVisibilityCount];
		RootSignatureType RootSignatureType = RootSignatureType::Raster;
		bool AllowIAInputLayout;
		bool AllowStreamOuput = false;

		inline bool operator==(const QuantizedBoundShaderState& rhs) const
		{
			return 0 == std::memcmp(this, &rhs, sizeof(rhs));
		}

		uint32 GetHashCode() const;
	};

	class RootSignature
	{
	public:
		RootSignature(const ShaderBlob& blob)
		{
			Init(blob);
		}
		RootSignature(const QuantizedBoundShaderState& QBSS)
		{
			Init(QBSS);
		}

		void Init(const ShaderBlob& blob);
		void Init(const QuantizedBoundShaderState& QBSS);
	};

	

	class RootSignatureMap
	{
	public:
		RootSignatureMap(ID3D12Device* pDevice)
			:Device(pDevice)
		{}

		RootSignature* GetRootSignature(const QuantizedBoundShaderState& QBSS);
	private:
		RootSignature* CreateRootSignature(const QuantizedBoundShaderState& QBSS);

		std::mutex mutex;

		struct KeyHash
		{
			leo::uint32 operator()(const QuantizedBoundShaderState& which) const
			{
				return which.GetHashCode();
			}
		};

		ID3D12Device* Device;

		leo::unordered_map< QuantizedBoundShaderState,leo::unique_ptr<RootSignature>, KeyHash, std::equal_to<QuantizedBoundShaderState>> Map;
	};
}

#endif
