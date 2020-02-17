#include "D3D12RayTracing.h"
#include "Context.h"

void RayTracingShaderTable::UploadToGPU(Windows::D3D12::Device* Device)
{

	if (!bIsDirty)
		return;

	Buffer = leo::share_raw(Device->CreateVertexBuffer(
		platform::Render::Buffer::Static,
		0,
		static_cast<uint32>(Data.size()),
		platform::Render::EF_Unknown,
		Data.data()
	));


	bIsDirty = false;
}

COMPtr<ID3D12StateObject> CreateRayTracingStateObject(ID3D12Device5* RayTracingDevice, const leo::span<const DXILLibrary*>& ShaderLibraries, const leo::span<LPCWSTR>& Exports, uint32 MaxPayloadSizeInBytes, const leo::span<const D3D12_HIT_GROUP_DESC>& HitGroups, const ID3D12RootSignature* GlobalRootSignature, const leo::span<ID3D12RootSignature*>& LocalRootSignatures, const leo::span<uint32>& LocalRootSignatureAssociations, const leo::span<D3D12_EXISTING_COLLECTION_DESC>& ExistingCollections, D3D12_STATE_OBJECT_TYPE StateObjectType)
{
	// There are several pipeline sub-objects that are always required:
	// 1) D3D12_RAYTRACING_SHADER_CONFIG
	// 2) D3D12_SUBOBJECT_TO_EXPORTS_ASSOCIATION
	// 3) D3D12_RAYTRACING_PIPELINE_CONFIG
	// 4) Global root signature
	static constexpr uint32 NumRequiredSubobjects = 4;

	leo::vector< D3D12_STATE_SUBOBJECT> Subobjects;
	Subobjects.reserve(NumRequiredSubobjects
		+ ShaderLibraries.size()
		+ HitGroups.size()
		+ LocalRootSignatures.size()
		+ Exports.size()
		+ ExistingCollections.size()
	);

	leo::vector<D3D12_SUBOBJECT_TO_EXPORTS_ASSOCIATION> ExportAssociations;
	ExportAssociations.reserve(Exports.size());
	auto NumExports = Exports.size();

	//Shader libraries
	for (auto& Library : ShaderLibraries)
	{
		Subobjects.emplace_back(Library->GetSubobject());
	}

	// Shader config

	D3D12_RAYTRACING_SHADER_CONFIG ShaderConfig = {};
	ShaderConfig.MaxAttributeSizeInBytes = 8; // sizeof 2 floats (barycentrics)
	ShaderConfig.MaxPayloadSizeInBytes = MaxPayloadSizeInBytes;
	const uint32 ShaderConfigIndex = static_cast<uint32>(Subobjects.size());
	Subobjects.push_back({ D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_SHADER_CONFIG, &ShaderConfig });

	D3D12_SUBOBJECT_TO_EXPORTS_ASSOCIATION ShaderConfigAssociation = {};
	ShaderConfigAssociation.NumExports = static_cast<UINT>(Exports.size());
	ShaderConfigAssociation.pExports = Exports.data();
	ShaderConfigAssociation.pSubobjectToAssociate = &Subobjects[ShaderConfigIndex];
	Subobjects.push_back({ D3D12_STATE_SUBOBJECT_TYPE_SUBOBJECT_TO_EXPORTS_ASSOCIATION, &ShaderConfigAssociation });

	for (auto& HitGroupDesc : HitGroups)
	{
		Subobjects.push_back({ D3D12_STATE_SUBOBJECT_TYPE_HIT_GROUP, &HitGroupDesc });
	}

	// Pipeline config

	D3D12_RAYTRACING_PIPELINE_CONFIG PipelineConfig = {};
	PipelineConfig.MaxTraceRecursionDepth = 1; // Only allow ray tracing from RayGen shader
	const uint32 PipelineConfigIndex = static_cast<uint32>(Subobjects.size());
	Subobjects.push_back({ D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_PIPELINE_CONFIG, &PipelineConfig });

	// Global root signature

	Subobjects.push_back({ D3D12_STATE_SUBOBJECT_TYPE_GLOBAL_ROOT_SIGNATURE, &GlobalRootSignature });

	// Local root signatures

	const uint32 LocalRootSignatureBaseIndex = static_cast<uint32>(Subobjects.size());
	for (int32 SignatureIndex = 0; SignatureIndex < LocalRootSignatures.size(); ++SignatureIndex)
	{
		Subobjects.push_back({ D3D12_STATE_SUBOBJECT_TYPE_LOCAL_ROOT_SIGNATURE, &LocalRootSignatures[SignatureIndex] });
	}

	// Local root signature associations

	for (int32 ExportIndex = 0; ExportIndex < Exports.size(); ++ExportIndex)
	{
		// If custom LocalRootSignatureAssociations data is not provided, then assume same default local RS association.
		const int32 LocalRootSignatureIndex = LocalRootSignatureAssociations.size() != 0
			? LocalRootSignatureAssociations[ExportIndex]
			: 0;

		D3D12_SUBOBJECT_TO_EXPORTS_ASSOCIATION& Association = ExportAssociations[ExportIndex];
		Association = D3D12_SUBOBJECT_TO_EXPORTS_ASSOCIATION{};
		Association.NumExports = 1;
		Association.pExports = &Exports[ExportIndex];

		Association.pSubobjectToAssociate = &Subobjects[LocalRootSignatureBaseIndex + LocalRootSignatureIndex];

		Subobjects.push_back({ D3D12_STATE_SUBOBJECT_TYPE_SUBOBJECT_TO_EXPORTS_ASSOCIATION, &ExportAssociations[ExportIndex] });
	}

	// Existing collection objects

	for (int32 CollectionIndex = 0; CollectionIndex < ExistingCollections.size(); ++CollectionIndex)
	{
		Subobjects.push_back({ D3D12_STATE_SUBOBJECT_TYPE_EXISTING_COLLECTION, &ExistingCollections[CollectionIndex] });
	}

	// Create ray tracing pipeline state object

	D3D12_STATE_OBJECT_DESC Desc = {};
	Desc.NumSubobjects = static_cast<UINT>(Subobjects.size());
	Desc.pSubobjects = &Subobjects[0];
	Desc.Type = StateObjectType;

	COMPtr<ID3D12StateObject> Result;
	CheckHResult(RayTracingDevice->CreateStateObject(&Desc, COMPtr_RefParam(Result, IID_ID3D12StateObject)));

	return Result;
}
