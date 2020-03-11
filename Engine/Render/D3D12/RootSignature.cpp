#include "RootSignature.h"
#include "../../Core/Hash/CityHash.h"
#include "Context.h"
#include "D3D12RayTracing.h"

using namespace platform_ex::Windows::D3D12;
using platform::Render::ShaderType;

namespace
{
	// Root parameter costs in DWORDs as described here: https://docs.microsoft.com/en-us/windows/desktop/direct3d12/root-signature-limits
	static const uint32 RootDescriptorTableCostGlobal = 1; // Descriptor tables cost 1 DWORD
	static const uint32 RootDescriptorTableCostLocal = 2; // Local root signature descriptor tables cost 2 DWORDs -- undocumented as of 2018-11-12
	static const uint32 RootConstantCost = 1; // Each root constant is 1 DWORD
	static const uint32 RootDescriptorCost = 2; // Root descriptor is 64-bit GPU virtual address, 2 DWORDs
}

FORCEINLINE D3D12_SHADER_VISIBILITY GetD3D12ShaderVisibility(uint8 Visibility)
{
	switch ((ShaderType)Visibility)
	{
	case ShaderType::VertexShader:
		return D3D12_SHADER_VISIBILITY_VERTEX;
	case ShaderType::HullShader:
		return D3D12_SHADER_VISIBILITY_HULL;
	case ShaderType::DomainShader:
		return D3D12_SHADER_VISIBILITY_DOMAIN;
	case ShaderType::GeometryShader:
		return D3D12_SHADER_VISIBILITY_GEOMETRY;
	case ShaderType::PixelShader:
		return D3D12_SHADER_VISIBILITY_PIXEL;
	case ShaderType::VisibilityAll:
		return D3D12_SHADER_VISIBILITY_ALL;

	default:
		lconstraint(false);
		return static_cast<D3D12_SHADER_VISIBILITY>(-1);
	};
}

FORCEINLINE D3D12_ROOT_SIGNATURE_FLAGS GetD3D12RootSignatureDenyFlag(uint8 Visibility)
{
	switch ((ShaderType)Visibility)
	{
	case ShaderType::VertexShader:
		return D3D12_ROOT_SIGNATURE_FLAG_DENY_VERTEX_SHADER_ROOT_ACCESS;
	case ShaderType::HullShader:
		return D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS;
	case ShaderType::DomainShader:
		return D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS;
	case  ShaderType::GeometryShader:
		return D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;
	case ShaderType::PixelShader:
		return D3D12_ROOT_SIGNATURE_FLAG_DENY_PIXEL_SHADER_ROOT_ACCESS;
	case ShaderType::VisibilityAll:
		return D3D12_ROOT_SIGNATURE_FLAG_NONE;

	default:
		lconstraint(false);
		return static_cast<D3D12_ROOT_SIGNATURE_FLAGS>(-1);
	};
}

void RootSignature::Init(const ShaderBlob& blob)
{
}

void platform_ex::Windows::D3D12::RootSignature::Init(const QuantizedBoundShaderState& QBSS,ID3D12Device* pDevice)
{
	const auto ResourceBindingTier = Context::Instance().GetDevice().GetResourceBindingTier();
	RootSignatureDesc Desc(QBSS, ResourceBindingTier);

	uint32 BindingSpace = 0; // Default binding space for D3D 11 & 12 shaders

	if (QBSS.RootSignatureType == RootSignatureType::RayTracingGlobal)
	{
		BindingSpace = RAY_TRACING_REGISTER_SPACE_GLOBAL;
	}
	else if (QBSS.RootSignatureType == RootSignatureType::RayTracingLocal)
	{
		BindingSpace = RAY_TRACING_REGISTER_SPACE_LOCAL;
	}

	Init(Desc.GetDesc(), BindingSpace,pDevice);
}

void RootSignature::Init(const D3D12_VERSIONED_ROOT_SIGNATURE_DESC& InDesc, uint32 BindingSpace, ID3D12Device* pDevice)
{
	const D3D_ROOT_SIGNATURE_VERSION MaxRootSignatureVersion = Context::Instance().GetDevice().GetRootSignatureVersion();

	COMPtr<ID3DBlob> Error;
	const HRESULT SerializeHR = D3DX12SerializeVersionedRootSignature(&InDesc, MaxRootSignatureVersion, &SignatureBlob.ReleaseAndGetRef(), &Error.ReleaseAndGetRef());

	if(Error.GetRef())
	{
		auto error = reinterpret_cast<char*>(Error->GetBufferPointer());
		LE_LogError(error);
	}

	CheckHResult(SerializeHR);

	CheckHResult(pDevice->CreateRootSignature(0,
		SignatureBlob->GetBufferPointer(),
		SignatureBlob->GetBufferSize(),
		COMPtr_RefParam(Signature,IID_ID3D12RootSignature)));

	AnalyzeSignature(InDesc, BindingSpace);
}

void RootSignature::AnalyzeSignature(const D3D12_VERSIONED_ROOT_SIGNATURE_DESC& Desc, uint32 BindingSpace)
{
	switch (Desc.Version)
	{
	case D3D_ROOT_SIGNATURE_VERSION_1_0:
		InternalAnalyzeSignature(Desc.Desc_1_0, BindingSpace);
		break;

	case D3D_ROOT_SIGNATURE_VERSION_1_1:
		InternalAnalyzeSignature(Desc.Desc_1_1, BindingSpace);
		break;

	default:
		LAssert(false, "Invalid root signature version");
		break;
	}
}

template<typename RootSignatureDescType>
void RootSignature::InternalAnalyzeSignature(const RootSignatureDescType& Desc, uint32 BindingSpace)
{
	std::memset(BindSlotMap, 0xFF, sizeof(BindSlotMap));
	TotalRootSignatureSizeInDWORDs = 0;

	const uint32 RootDescriptorTableCost = (Desc.Flags & D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE) ? RootDescriptorTableCostLocal : RootDescriptorTableCostGlobal;

	// Go through each root parameter.
	for (uint32 i = 0; i < Desc.NumParameters; i++)
	{
		const auto& CurrentParameter = Desc.pParameters[i];

		uint32 ParameterBindingSpace = ~0u;

		switch (CurrentParameter.ParameterType)
		{
		case D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE:
			lconstraint(CurrentParameter.DescriptorTable.NumDescriptorRanges == 1); // Code currently assumes a single descriptor range.
			ParameterBindingSpace = CurrentParameter.DescriptorTable.pDescriptorRanges[0].RegisterSpace;
			TotalRootSignatureSizeInDWORDs += RootDescriptorTableCost;
			break;
		case D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS:
			ParameterBindingSpace = CurrentParameter.Constants.RegisterSpace;
			TotalRootSignatureSizeInDWORDs += RootConstantCost * CurrentParameter.Constants.Num32BitValues;
			break;
		case D3D12_ROOT_PARAMETER_TYPE_CBV:
		case D3D12_ROOT_PARAMETER_TYPE_SRV:
		case D3D12_ROOT_PARAMETER_TYPE_UAV:
			ParameterBindingSpace = CurrentParameter.Descriptor.RegisterSpace;
			TotalRootSignatureSizeInDWORDs += RootDescriptorCost;
			break;
		default:
			lassume(false);
			break;
		}

		if (ParameterBindingSpace != BindingSpace)
		{
			// Only consider parameters in the requested binding space.
			continue;
		}

		ShaderType CurrentVisibleSF = ShaderType::NumStandardType;
		switch (CurrentParameter.ShaderVisibility)
		{
		case D3D12_SHADER_VISIBILITY_ALL:
			CurrentVisibleSF = ShaderType::VisibilityAll;
			break;

		case D3D12_SHADER_VISIBILITY_VERTEX:
			CurrentVisibleSF = ShaderType::VertexShader;
			break;
		case D3D12_SHADER_VISIBILITY_HULL:
			CurrentVisibleSF = ShaderType::HullShader;
			break;
		case D3D12_SHADER_VISIBILITY_DOMAIN:
			CurrentVisibleSF = ShaderType::DomainShader;
			break;
		case D3D12_SHADER_VISIBILITY_GEOMETRY:
			CurrentVisibleSF = ShaderType::GeometryShader;
			break;
		case D3D12_SHADER_VISIBILITY_PIXEL:
			CurrentVisibleSF = ShaderType::PixelShader;
			break;

		default:
			lconstraint(false);
			break;
		}

		// Determine shader resource counts.
		{
			switch (CurrentParameter.ParameterType)
			{
			case D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE:
				lconstraint(CurrentParameter.DescriptorTable.NumDescriptorRanges == 1);	// Code currently assumes a single descriptor range.
				{
					const auto& CurrentRange = CurrentParameter.DescriptorTable.pDescriptorRanges[0];
					lconstraint(CurrentRange.BaseShaderRegister == 0);	// Code currently assumes always starting at register 0.
					lconstraint(CurrentRange.RegisterSpace == BindingSpace); // Parameters in other binding spaces are expected to be filtered out at this point

					switch (CurrentRange.RangeType)
					{
					case D3D12_DESCRIPTOR_RANGE_TYPE_SRV:
						//SetMaxSRVCount(CurrentVisibleSF, CurrentRange.NumDescriptors);
						SetSRVRDTBindSlot(CurrentVisibleSF, i);
						break;
					case D3D12_DESCRIPTOR_RANGE_TYPE_UAV:
						//SetMaxUAVCount(CurrentVisibleSF, CurrentRange.NumDescriptors);
						SetUAVRDTBindSlot(CurrentVisibleSF, i);
						break;
					case D3D12_DESCRIPTOR_RANGE_TYPE_CBV:
						//IncrementMaxCBVCount(CurrentVisibleSF, CurrentRange.NumDescriptors);
						SetCBVRDTBindSlot(CurrentVisibleSF, i);
						//UpdateCBVRegisterMaskWithDescriptorRange(CurrentVisibleSF, CurrentRange);
						break;
					case D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER:
						//SetMaxSamplerCount(CurrentVisibleSF, CurrentRange.NumDescriptors);
						SetSamplersRDTBindSlot(CurrentVisibleSF, i);
						break;

					default: lconstraint(false); break;
					}
				}
				break;

			case D3D12_ROOT_PARAMETER_TYPE_CBV:
			{
				lconstraint(CurrentParameter.Descriptor.RegisterSpace == BindingSpace); // Parameters in other binding spaces are expected to be filtered out at this point

				//IncrementMaxCBVCount(CurrentVisibleSF, 1);
				if (CurrentParameter.Descriptor.ShaderRegister == 0)
				{
					// This is the first CBV for this stage, save it's root parameter index (other CBVs will be indexed using this base root parameter index).
					SetCBVRDBindSlot(CurrentVisibleSF, i);
				}

				//UpdateCBVRegisterMaskWithDescriptor(CurrentVisibleSF, CurrentParameter.Descriptor);

				// The first CBV for this stage must come first in the root signature, and subsequent root CBVs for this stage must be contiguous.
				lconstraint(0xFF != CBVRDBindSlot(CurrentVisibleSF, 0));
				lconstraint(i == CBVRDBindSlot(CurrentVisibleSF, 0) + CurrentParameter.Descriptor.ShaderRegister);
			}
			break;

			default:
				// Need to update this for the other types. Currently we only use descriptor tables in the root signature.
				lconstraint(false);
				break;
			}
		}
	}

}

leo::uint32 QuantizedBoundShaderState::GetHashCode() const
{
	return static_cast<leo::uint32>(CityHash64((const char*)this,sizeof(*this)));
}

RootSignature* platform_ex::Windows::D3D12::RootSignatureMap::GetRootSignature(const QuantizedBoundShaderState& QBSS)
{
	std::lock_guard lock{ mutex };

	auto iter = Map.find(QBSS);
	if (iter == Map.end())
	{
		return CreateRootSignature(QBSS);
	}

	return iter->second.get();
}

RootSignature* platform_ex::Windows::D3D12::RootSignatureMap::CreateRootSignature(const QuantizedBoundShaderState& QBSS)
{
	auto pair = Map.emplace(QBSS, std::make_unique<RootSignature>(QBSS,Device));

	return pair.first->second.get();
}

platform_ex::Windows::D3D12::RootSignatureDesc::RootSignatureDesc(const QuantizedBoundShaderState& QBSS, const D3D12_RESOURCE_BINDING_TIER ResourceBindingTier)
	:RootParametersSize(0)
{
	const uint8 ShaderVisibilityPriorityOrder[] = { (uint8)ShaderType::PixelShader, (uint8)ShaderType::VertexShader, (uint8)ShaderType::GeometryShader, (uint8)ShaderType::HullShader, (uint8)ShaderType::DomainShader, 5 };
	const D3D12_ROOT_PARAMETER_TYPE RootParameterTypePriorityOrder[] = { D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE, D3D12_ROOT_PARAMETER_TYPE_CBV };
	uint32 RootParameterCount = 0;

	// Determine if our descriptors or their data is static based on the resource binding tier.
	// We do this because sometimes (based on binding tier) our descriptor tables are bigger than the # of descriptors we copy. See FD3D12QuantizedBoundShaderState::InitShaderRegisterCounts().
	const D3D12_DESCRIPTOR_RANGE_FLAGS SRVDescriptorRangeFlags = (ResourceBindingTier <= D3D12_RESOURCE_BINDING_TIER_1) ?
		D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC_WHILE_SET_AT_EXECUTE :
		D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC_WHILE_SET_AT_EXECUTE | D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE;

	const D3D12_DESCRIPTOR_RANGE_FLAGS CBVDescriptorRangeFlags = (ResourceBindingTier <= D3D12_RESOURCE_BINDING_TIER_2) ?
		D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC_WHILE_SET_AT_EXECUTE :
		D3D12_DESCRIPTOR_RANGE_FLAG_DATA_STATIC_WHILE_SET_AT_EXECUTE | D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE;

	const D3D12_DESCRIPTOR_RANGE_FLAGS UAVDescriptorRangeFlags = (ResourceBindingTier <= D3D12_RESOURCE_BINDING_TIER_2) ?
		D3D12_DESCRIPTOR_RANGE_FLAG_DATA_VOLATILE :
		D3D12_DESCRIPTOR_RANGE_FLAG_DATA_VOLATILE | D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE;

	const D3D12_DESCRIPTOR_RANGE_FLAGS TextureSampleDescriptorRangeFlags = (ResourceBindingTier <= D3D12_RESOURCE_BINDING_TIER_1) ?
		D3D12_DESCRIPTOR_RANGE_FLAG_NONE :
		D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE;

	const D3D12_ROOT_DESCRIPTOR_FLAGS CBVRootDescriptorFlags = D3D12_ROOT_DESCRIPTOR_FLAG_DATA_STATIC;	// We always set the data in an upload heap before calling Set*RootConstantBufferView.

	uint32 BindingSpace = 0; // Default binding space for D3D 11 & 12 shaders

	if (QBSS.RootSignatureType == RootSignatureType::RayTracingLocal)
	{
		BindingSpace = RAY_TRACING_REGISTER_SPACE_LOCAL;

		// Add standard root parameters for hit groups, as per HitGroupSystemParameters declaration in D3D12RayTracing.h and RayTracingHitGroupCommon.lsl:
		//          Resources:
		// 8 bytes: index buffer as root SRV (raw buffer)
		// 8 bytes: vertex buffer as root SRV (raw buffer)
		//          HitGroupSystemRootConstants:
		// 4 bytes: index/vertex fetch configuration as root constant (bitfield defining index and vertex formats)
		// 4 bytes: index buffer offset in bytes
		// 4 bytes: hit group user data
		// 4 bytes: unused padding to ensure the next parameter is aligned to 8-byte boundary
		// -----------
		// 32 bytes

		lconstraint(RootParameterCount == 0 && RootParametersSize == 0); // We expect system RT parameters to come first

		// Index buffer descriptor
		{
			lconstraint(RootParameterCount < MaxRootParameters);
			TableSlots[RootParameterCount].InitAsShaderResourceView(RAY_TRACING_SYSTEM_INDEXBUFFER_REGISTER, RAY_TRACING_REGISTER_SPACE_SYSTEM);
			RootParameterCount++;
			RootParametersSize += RootDescriptorCost;
		}

		// Vertex buffer descriptor
		{
			lconstraint(RootParameterCount < MaxRootParameters);
			TableSlots[RootParameterCount].InitAsShaderResourceView(RAY_TRACING_SYSTEM_VERTEXBUFFER_REGISTER, RAY_TRACING_REGISTER_SPACE_SYSTEM);
			RootParameterCount++;
			RootParametersSize += RootDescriptorCost;
		}

		// HitGroupSystemRootConstants structure
		{
			lconstraint(RootParameterCount < MaxRootParameters);
			static_assert(sizeof(HitGroupSystemRootConstants) % 8 == 0, "FHitGroupSystemRootConstants structure must be 8-byte aligned");
			const uint32 NumConstants = sizeof(HitGroupSystemRootConstants) / sizeof(uint32);
			TableSlots[RootParameterCount].InitAsConstants(NumConstants, RAY_TRACING_SYSTEM_ROOTCONSTANT_REGISTER, RAY_TRACING_REGISTER_SPACE_SYSTEM);
			RootParameterCount++;
			RootParametersSize += NumConstants * RootConstantCost;
		}
	}
	else if (QBSS.RootSignatureType == RootSignatureType::RayTracingGlobal)
	{
		BindingSpace = RAY_TRACING_REGISTER_SPACE_GLOBAL;
	}

	const uint32 RootDescriptorTableCost = QBSS.RootSignatureType == RootSignatureType::RayTracingLocal ? RootDescriptorTableCostLocal : RootDescriptorTableCostGlobal;

	// For each root parameter type...
	for (uint32 RootParameterTypeIndex = 0; RootParameterTypeIndex < leo::size(RootParameterTypePriorityOrder); RootParameterTypeIndex++)
	{
		const D3D12_ROOT_PARAMETER_TYPE& RootParameterType = RootParameterTypePriorityOrder[RootParameterTypeIndex];

		// ... and each shader stage visibility ...
		for (uint32 ShaderVisibilityIndex = 0; ShaderVisibilityIndex < leo::size(ShaderVisibilityPriorityOrder); ShaderVisibilityIndex++)
		{
			const auto& Visibility = ShaderVisibilityPriorityOrder[ShaderVisibilityIndex];
			const auto& Shader = QBSS.RegisterCounts[Visibility];

			switch (RootParameterType)
			{
			case D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE:
			{
				if (Shader.NumSRVs > 0)
				{
					lconstraint(RootParameterCount < MaxRootParameters);
					DescriptorRanges[RootParameterCount].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, Shader.NumSRVs, 0u, BindingSpace, SRVDescriptorRangeFlags);
					TableSlots[RootParameterCount].InitAsDescriptorTable(1, &DescriptorRanges[RootParameterCount], GetD3D12ShaderVisibility(Visibility));
					RootParameterCount++;
					RootParametersSize += RootDescriptorTableCost;
				}

				if (Shader.NumCBs > MAX_ROOT_CBVS)
				{
					LAssert(QBSS.RootSignatureType != RootSignatureType::RayTracingLocal, "CBV descriptor tables are not implemented for local root signatures");

					// Use a descriptor table for the 'excess' CBVs
					lconstraint(RootParameterCount < MaxRootParameters);
					DescriptorRanges[RootParameterCount].Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, Shader.NumCBs - MAX_ROOT_CBVS, MAX_ROOT_CBVS, BindingSpace, CBVDescriptorRangeFlags);
					TableSlots[RootParameterCount].InitAsDescriptorTable(1, &DescriptorRanges[RootParameterCount], GetD3D12ShaderVisibility(Visibility));
					RootParameterCount++;
					RootParametersSize += RootDescriptorTableCost;
				}

				if (Shader.NumSamplers > 0)
				{
					lconstraint(RootParameterCount < MaxRootParameters);
					DescriptorRanges[RootParameterCount].Init(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, Shader.NumSamplers, 0u, BindingSpace, TextureSampleDescriptorRangeFlags);
					TableSlots[RootParameterCount].InitAsDescriptorTable(1, &DescriptorRanges[RootParameterCount], GetD3D12ShaderVisibility(Visibility));
					RootParameterCount++;
					RootParametersSize += RootDescriptorTableCost;
				}

				if (Shader.NumUAVs > 0)
				{
					lconstraint(RootParameterCount < MaxRootParameters);
					DescriptorRanges[RootParameterCount].Init(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, Shader.NumUAVs, 0u, BindingSpace, UAVDescriptorRangeFlags);
					TableSlots[RootParameterCount].InitAsDescriptorTable(1, &DescriptorRanges[RootParameterCount], GetD3D12ShaderVisibility(Visibility));
					RootParameterCount++;
					RootParametersSize += RootDescriptorTableCost;
				}
				break;
			}

			case D3D12_ROOT_PARAMETER_TYPE_CBV:
			{
				for (uint32 ShaderRegister = 0; (ShaderRegister < Shader.NumCBs) && (ShaderRegister < MAX_ROOT_CBVS); ShaderRegister++)
				{
					lconstraint(RootParameterCount < MaxRootParameters);
					TableSlots[RootParameterCount].InitAsConstantBufferView(ShaderRegister, BindingSpace, CBVRootDescriptorFlags, GetD3D12ShaderVisibility(Visibility));
					RootParameterCount++;
					RootParametersSize += RootDescriptorCost;
				}
				break;
			}

			default:
				lconstraint(false);
				break;
			}
		}
	}

	D3D12_ROOT_SIGNATURE_FLAGS Flags = D3D12_ROOT_SIGNATURE_FLAG_NONE;

	if (QBSS.RootSignatureType == RootSignatureType::RayTracingLocal)
	{
		Flags = D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE;
	}
	else if (QBSS.RootSignatureType == RootSignatureType::Raster)
	{
		// Determine what shader stages need access in the root signature.

		if (QBSS.AllowIAInputLayout)
		{
			Flags |= D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
		}
		if (QBSS.AllowStreamOuput)
		{
			Flags |= D3D12_ROOT_SIGNATURE_FLAG_ALLOW_STREAM_OUTPUT;
		}

		for (uint32 ShaderVisibilityIndex = 0; ShaderVisibilityIndex < leo::size(ShaderVisibilityPriorityOrder); ShaderVisibilityIndex++)
		{
			const auto& Visibility = ShaderVisibilityPriorityOrder[ShaderVisibilityIndex];
			const auto& Shader = QBSS.RegisterCounts[Visibility];
			if ((Shader.NumSRVs == 0) &&
				(Shader.NumCBs == 0) &&
				(Shader.NumUAVs == 0) &&
				(Shader.NumSamplers == 0))
			{
				// This shader stage doesn't use any descriptors, deny access to the shader stage in the root signature.
				Flags = (Flags | GetD3D12RootSignatureDenyFlag(Visibility));
			}
		}
	}

	RootDesc.Init_1_1(RootParameterCount, TableSlots, 0, nullptr, Flags);
}
