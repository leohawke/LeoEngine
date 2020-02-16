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

	class RootSignatureDesc
	{
	public:
		explicit RootSignatureDesc(const QuantizedBoundShaderState& QBSS, const D3D12_RESOURCE_BINDING_TIER ResourceBindingTier);

		inline const D3D12_VERSIONED_ROOT_SIGNATURE_DESC& GetDesc() const { return RootDesc; }

		static constexpr uint32 MaxRootParameters = 32;	// Arbitrary max, increase as needed.
	private:

		uint32 RootParametersSize;	// The size of all root parameters in the root signature. Size in DWORDs, the limit is 64.
		CD3DX12_ROOT_PARAMETER1 TableSlots[MaxRootParameters];
		CD3DX12_DESCRIPTOR_RANGE1 DescriptorRanges[MaxRootParameters];
		CD3DX12_VERSIONED_ROOT_SIGNATURE_DESC RootDesc;
	};


	class RootSignature
	{
	public:
		RootSignature(const ShaderBlob& blob)
		{
			Init(blob);
		}
		RootSignature(const QuantizedBoundShaderState& QBSS,ID3D12Device* pDevice)
		{
			Init(QBSS,pDevice);
		}

		void Init(const ShaderBlob& blob);
		void Init(const QuantizedBoundShaderState& QBSS,ID3D12Device* pDevice);
		void Init(const D3D12_VERSIONED_ROOT_SIGNATURE_DESC& InDesc, uint32 BindingSpace, ID3D12Device* pDevice);

		uint32 GetTotalRootSignatureSizeInBytes() const { return 4 * TotalRootSignatureSizeInDWORDs; }
	private:
		void AnalyzeSignature(const D3D12_VERSIONED_ROOT_SIGNATURE_DESC& Desc, uint32 BindingSpace);
		template<typename RootSignatureDescType>
		void InternalAnalyzeSignature(const RootSignatureDescType& Desc, uint32 BindingSpace);

	private:
		COMPtr<ID3DBlob> SignatureBlob;

	public:
		COMPtr<ID3D12RootSignature> Signature;
		uint8 TotalRootSignatureSizeInDWORDs = 0;
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
