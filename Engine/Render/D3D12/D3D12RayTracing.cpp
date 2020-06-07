#include "D3D12RayTracing.h"
#include "Context.h"
#include "RayTracingPipelineState.h"
#include "../../Core/Hash/CityHash.h"
#include "ResourceView.h"
#include "Texture.h"
#include "GraphicsBuffer.hpp"
#include <LFramework/Core/LString.h>

using namespace platform_ex::Windows::D3D12;

using platform::Render::ShaderType;

void RayTracingShaderTable::UploadToGPU(Windows::D3D12::Device* Device)
{

	if (!bIsDirty)
		return;

	Buffer = leo::share_raw(Device->CreateVertexBuffer(
		platform::Render::Buffer::Static,
		platform::Render::EAccessHint::EA_GPURead,
		static_cast<uint32>(Data.size()),
		platform::Render::EF_Unknown,
		Data.data()
	));

	Buffer->SetName("RayTracingShaderTable::Buffer");

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
	ExportAssociations.resize(Exports.size());
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



RayTracingDescriptorHeapCache::~RayTracingDescriptorHeapCache()
{
	std::unique_lock Lock{ CriticalSection };

	for (auto& It : Entries)
	{
		It.Heap->Release();
	}
	Entries.clear();
}

void RayTracingDescriptorHeapCache::ReleaseHeap(Entry& Entry)
{
	std::unique_lock Lock{ CriticalSection };

	Entries.emplace_back(Entry);

	--AllocatedEntries;
}

RayTracingDescriptorHeapCache::Entry RayTracingDescriptorHeapCache::AllocateHeap(D3D12_DESCRIPTOR_HEAP_TYPE Type, uint32 NumDescriptors)
{
	std::unique_lock Lock{ CriticalSection };

	++AllocatedEntries;

	Entry Result = {};

	auto& Fence = Device->GetFence();
	auto CompletedFenceValue = Fence.GetLastCompletedFenceFast();

	for (unsigned EntryIndex = 0; EntryIndex < Entries.size(); ++EntryIndex)
	{
		auto& It = Entries[EntryIndex];

		if (It.Type == Type && It.NumDescriptors >= NumDescriptors && It.FenceValue <= CompletedFenceValue)
		{
			Result = It;
			Entries[EntryIndex] = Entries.back();
			Entries.pop_back();

			return Result;
		}
	}

	// Compatible heap was not found in cache, so create a new one.
	ReleaseStaleEntries(100, CompletedFenceValue); // Release heaps that were not used for 100 frames before allocating new.

	D3D12_DESCRIPTOR_HEAP_DESC Desc = {};

	Desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	Desc.Type = Type;
	Desc.NumDescriptors = NumDescriptors;
	Desc.NodeMask = 0;

	ID3D12DescriptorHeap* D3D12Heap = nullptr;
	Device->GetRayTracingDevice()->CreateDescriptorHeap(&Desc, IID_PPV_ARGS(&D3D12Heap));
	Windows::D3D::Debug(D3D12Heap, Desc.Type == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV ? "RT View Heap" : "RT Sampler Heap");

	Result.NumDescriptors = NumDescriptors;
	Result.Type = Type;
	Result.Heap = D3D12Heap;

	return Result;
}

void RayTracingDescriptorHeapCache::ReleaseStaleEntries(uint32 MaxAge, uint64 CompletedFenceValue)
{
	unsigned EntryIndex = 0;
	while (EntryIndex < Entries.size())
	{
		auto& It = Entries[EntryIndex];
		if (It.FenceValue + MaxAge <= CompletedFenceValue)
		{
			It.Heap->Release();
			Entries[EntryIndex] = Entries.back();
			Entries.pop_back();
		}
		else
		{
			++EntryIndex;
		}
	}
}

RayTracingDescriptorHeap::~RayTracingDescriptorHeap()
{
	if (D3D12Heap)
	{
		Device->GetRayTracingDescriptorHeapCache()->ReleaseHeap(HeapCacheEntry);
	}
}

void RayTracingDescriptorHeap::Init(uint32 InMaxNumDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE Type)
{
	HeapCacheEntry = Device->GetRayTracingDescriptorHeapCache()->AllocateHeap(Type, InMaxNumDescriptors);

	MaxNumDescriptors = HeapCacheEntry.NumDescriptors;
	D3D12Heap = HeapCacheEntry.Heap;

	CPUBase = D3D12Heap->GetCPUDescriptorHandleForHeapStart();
	GPUBase = D3D12Heap->GetGPUDescriptorHandleForHeapStart();

	DescriptorSize = Device->GetRayTracingDevice()->GetDescriptorHandleIncrementSize(Type);
}

bool RayTracingDescriptorHeap::CanAllocate(uint32 InNumDescriptors) const
{
	return NumAllocatedDescriptors + InNumDescriptors <= MaxNumDescriptors;
}

uint32 RayTracingDescriptorHeap::Allocate(uint32 InNumDescriptors)
{
	lconstraint(CanAllocate(InNumDescriptors));

	uint32 Result = NumAllocatedDescriptors;
	NumAllocatedDescriptors += InNumDescriptors;
	return Result;
}

void RayTracingDescriptorHeap::UpdateSyncPoint()
{
	auto& Fence = Device->GetFence();

	HeapCacheEntry.FenceValue = std::max(HeapCacheEntry.FenceValue, Fence.GetCurrentFence());
}

void RayTracingDescriptorCache::SetDescriptorHeaps(D3D12RayContext& Context)
{
	ID3D12DescriptorHeap* Heaps[2] =
	{
		ViewHeap.D3D12Heap,
		SamplerHeap.D3D12Heap
	};

	Context.RayTracingCommandList()->SetDescriptorHeaps(2, Heaps);
}

uint32 RayTracingDescriptorCache::GetDescriptorTableBaseIndex(const D3D12_CPU_DESCRIPTOR_HANDLE* Descriptors, uint32 NumDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE Type)
{
	auto& Heap = (Type == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) ? ViewHeap : SamplerHeap;
	auto& Map = (Type == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV) ? ViewDescriptorTableCache : TextureSampleDescriptorTableCache;

	const uint64 Key = CityHash64((const char*)Descriptors, sizeof(Descriptors[0]) * NumDescriptors);

	uint32 DescriptorTableBaseIndex = ~0u;
	auto itr = Map.find(Key);

	if (itr != Map.end())
	{
		DescriptorTableBaseIndex = itr->second;
	}
	else
	{
		DescriptorTableBaseIndex = Heap.Allocate(NumDescriptors);

		D3D12_CPU_DESCRIPTOR_HANDLE DestDescriptor = Heap.GetDescriptorCPU(DescriptorTableBaseIndex);
		LAssert(Heap.CPUBase.ptr, "Ray tracing descriptor heap of type assigned to descriptor cache is invalid.");
		Device->GetRayTracingDevice()->CopyDescriptors(1, &DestDescriptor, &NumDescriptors, NumDescriptors, Descriptors, nullptr, Type);

		Map.emplace(Key, DescriptorTableBaseIndex);
	}

	return DescriptorTableBaseIndex;
}

inline leo::Text::String GenerateShaderName(const char* Prefix, uint64 Hash)
{
	return leo::sfmt("%s_%016llx", Prefix, Hash);
}

class ShaderCompile
{
public:
	using ECollectionType = RayTracingPipelineCache::CollectionType;

	ShaderCompile(
		RayTracingPipelineCache::Entry& InEntry,
		RayTracingPipelineCache::Key InCacheKey,
		ID3D12Device5* InRayTracingDevice,
		ECollectionType InCollectionType)
		: Entry(InEntry)
		, CacheKey(InCacheKey)
		, RayTracingDevice(InRayTracingDevice)
		, CollectionType(InCollectionType)
	{
	}
	
	void Do()
	{
		auto Shader = Entry.Shader.get();


		static constexpr uint32 MaxEntryPoints = 3; // CHS+AHS+IS for HitGroup or just a single entry point for other collection types
		std::vector<LPCWSTR> OriginalEntryPoints;
		std::vector<LPCWSTR> RenamedEntryPoints;

		const uint32 NumHitGroups = CollectionType == ECollectionType::HitGroup ? 1 : 0;
		const uint64 ShaderHash = CacheKey.ShaderHash;
		ID3D12RootSignature* GlobalRootSignature = CacheKey.GlobalRootSignature;
		ID3D12RootSignature* LocalRootSignature = CacheKey.LocalRootSignature;
		const uint32 DefaultLocalRootSignatureIndex = 0;
		uint32 MaxPayloadSizeInBytes = CacheKey.MaxPayloadSizeInBytes;

		D3D12_HIT_GROUP_DESC HitGroupDesc = {};

		if (CollectionType == ECollectionType::HitGroup)
		{
			HitGroupDesc.HitGroupExport = Entry.GetPrimaryExportNameChars();
			HitGroupDesc.Type = Shader->IntersectionEntryPoint.empty() ? D3D12_HIT_GROUP_TYPE_TRIANGLES : D3D12_HIT_GROUP_TYPE_PROCEDURAL_PRIMITIVE;

			{
				const auto& ExportName = Entry.ExportNames.emplace_back(GenerateShaderName("CHS", ShaderHash));

				HitGroupDesc.ClosestHitShaderImport = reinterpret_cast<LPCWSTR>(ExportName.c_str());

				OriginalEntryPoints.push_back(reinterpret_cast<LPCWSTR>(Shader->EntryPoint.c_str()));
				RenamedEntryPoints.push_back(reinterpret_cast<LPCWSTR>(ExportName.c_str()));
			}

			if (!Shader->AnyHitEntryPoint.empty())
			{
				const auto& ExportName = Entry.ExportNames.emplace_back(GenerateShaderName("AHS", ShaderHash));

				HitGroupDesc.AnyHitShaderImport = reinterpret_cast<LPCWSTR>(ExportName.c_str());

				OriginalEntryPoints.push_back(reinterpret_cast<LPCWSTR>(Shader->AnyHitEntryPoint.c_str()));
				RenamedEntryPoints.push_back(reinterpret_cast<LPCWSTR>(ExportName.c_str()));
			}

			if (!Shader->IntersectionEntryPoint.empty())
			{
				const auto& ExportName = Entry.ExportNames.emplace_back(GenerateShaderName("IS", ShaderHash));

				HitGroupDesc.IntersectionShaderImport = reinterpret_cast<LPCWSTR>(ExportName.c_str());

				OriginalEntryPoints.push_back(reinterpret_cast<LPCWSTR>(Shader->IntersectionEntryPoint.c_str()));
				RenamedEntryPoints.push_back(reinterpret_cast<LPCWSTR>(ExportName.c_str()));
			}
		}
		else
		{
			LAssert(CollectionType == ECollectionType::Miss || CollectionType == ECollectionType::RayGen || CollectionType == ECollectionType::Callable, "Unexpected RT sahder collection type");

			OriginalEntryPoints.push_back(reinterpret_cast<LPCWSTR>(Shader->EntryPoint.c_str()));
			RenamedEntryPoints.push_back(Entry.GetPrimaryExportNameChars());
		}


		for (const auto& ExportName : Entry.ExportNames)
		{
			D3D12_EXPORT_DESC ExportDesc = {};
			ExportDesc.Name = reinterpret_cast<LPCWSTR>(ExportName.c_str());
			ExportDesc.Flags = D3D12_EXPORT_FLAG_NONE;
			Entry.ExportDescs.push_back(ExportDesc);
		}

		DXILLibrary Library(Shader->ShaderByteCode.first.get(),Shader->ShaderByteCode.second,
			OriginalEntryPoints.data(),RenamedEntryPoints.data(),OriginalEntryPoints.size());

		const DXILLibrary* LibraryPtr = &Library;
		Entry.StateObject = CreateRayTracingStateObject(
			RayTracingDevice,
			leo::make_span(&LibraryPtr, 1),
			leo::make_span(RenamedEntryPoints),
			MaxPayloadSizeInBytes,
			leo::make_span(&HitGroupDesc, NumHitGroups),
			GlobalRootSignature,
			leo::make_span(&LocalRootSignature, 1),
			{},
			{},
			D3D12_STATE_OBJECT_TYPE_COLLECTION
		);
	}

	RayTracingPipelineCache::Entry& Entry;
	RayTracingPipelineCache::Key CacheKey;
	ID3D12Device5* RayTracingDevice;
	RayTracingPipelineCache::CollectionType CollectionType;
};

RayTracingPipelineCache::RayTracingPipelineCache(ID3D12Device5* Device)
{
	D3D12_VERSIONED_ROOT_SIGNATURE_DESC LocalRootSignatureDesc = {};
	LocalRootSignatureDesc.Version = D3D_ROOT_SIGNATURE_VERSION_1_0;
	LocalRootSignatureDesc.Desc_1_0.Flags |= D3D12_ROOT_SIGNATURE_FLAG_LOCAL_ROOT_SIGNATURE;
	DefaultLocalRootSignature.Init(LocalRootSignatureDesc,0, Device);
}

RayTracingPipelineCache::Entry* RayTracingPipelineCache::GetOrCompileShader(ID3D12Device5* RayTracingDevice, D3D12RayTracingShader* Shader, ID3D12RootSignature* GlobalRootSignature, uint32 MaxPayloadSizeInBytes, CollectionType CollectionType)
{
	std::unique_lock Lock{ CriticalSection };

	auto ShaderHash = reinterpret_cast<intptr_t>(Shader);
	ID3D12RootSignature* LocalRootSignature = nullptr;
	if (CollectionType == CollectionType::HitGroup || CollectionType == CollectionType::Callable)
	{
		// Only hit group and callable shaders have a local root signature
		LocalRootSignature = Shader->pRootSignature->GetSignature();
	}
	else
	{
		// ... all other shaders share a default empty local root signature
		LocalRootSignature = DefaultLocalRootSignature.GetSignature();
	}

	Key CacheKey;
	CacheKey.ShaderHash = ShaderHash;
	CacheKey.MaxPayloadSizeInBytes = MaxPayloadSizeInBytes;
	CacheKey.GlobalRootSignature = GlobalRootSignature;
	CacheKey.LocalRootSignature = LocalRootSignature;

	auto& FindResult = Cache[CacheKey];
	if (!FindResult)
	{
		FindResult = new Entry();

		auto& Entry = *FindResult;

		Entry.Shader =leo::make_observer(Shader);

		Entry.ExportNames.push_back(GenerateShaderName(GetCollectionTypeName(CollectionType), ShaderHash));

		ShaderCompile(Entry, CacheKey, RayTracingDevice, CollectionType).Do();
	}

	return FindResult;
}

struct FD3D12RayTracingGlobalResourceBinder
{
	FD3D12RayTracingGlobalResourceBinder(D3D12RayContext& InCommandContext)
		: CommandContext(InCommandContext)
	{
	}

	void SetRootCBV(uint32 BaseSlotIndex, uint32 DescriptorIndex, D3D12_GPU_VIRTUAL_ADDRESS Address)
	{
		CommandContext.RayTracingCommandList()->SetComputeRootConstantBufferView(BaseSlotIndex + DescriptorIndex, Address);
	}

	void SetRootSRV(uint32 BaseSlotIndex, uint32 DescriptorIndex, D3D12_GPU_VIRTUAL_ADDRESS Address)
	{
		CommandContext.RayTracingCommandList()->SetComputeRootShaderResourceView(BaseSlotIndex + DescriptorIndex, Address);
	}

	void SetRootDescriptorTable(uint32 SlotIndex, D3D12_GPU_DESCRIPTOR_HANDLE DescriptorTable)
	{
		CommandContext.RayTracingCommandList()->SetComputeRootDescriptorTable(SlotIndex, DescriptorTable);
	}

	D3D12_GPU_VIRTUAL_ADDRESS CreateTransientConstantBuffer(const void* Data, uint32 DataSize)
	{
		LAssert(0, "Loose parameters and transient constant buffers are not implemented for global ray tracing shaders (raygen, miss, callable)");
		return (D3D12_GPU_VIRTUAL_ADDRESS)0;
	}

	D3D12RayContext& CommandContext;
};

using platform::Render::Texture;
using platform::Render::GraphicsBuffer;
using platform::Render::TextureSampleDesc;
using platform::Render::ShaderResourceView;
using platform::Render::UnorderedAccessView;

using D3D12ShaderResourceView = Windows::D3D12::ShaderResourceView;
using D3D12UnorderedAccessView = Windows::D3D12::UnorderedAccessView;
using D3D12Texture = Windows::D3D12::Texture;
using D3D12ResourceHolder = Windows::D3D12::ResourceHolder;
using D3D12GraphicsBuffer = Windows::D3D12::GraphicsBuffer;

template <typename ResourceBinderType>
static void SetRayTracingShaderResources(
	D3D12RayContext& CommandContext,
	const platform_ex::Windows::D3D12::RayTracingShader* Shader,
	uint32 InNumTextures, Texture* const* Textures,
	uint32 InNumSRVs, ShaderResourceView* const* SRVs,
	uint32 InNumUniformBuffers, GraphicsBuffer* const* UniformBuffers,
	uint32 InNumSamplers, TextureSampleDesc* const* Samplers,
	uint32 InNumUAVs, UnorderedAccessView* const* UAVs,
	uint32 InLooseParameterDataSize, const void* InLooseParameterData,
	RayTracingDescriptorCache& DescriptorCache,
	ResourceBinderType& Binder)
{
	ID3D12Device* Device = CommandContext.GetDevice().GetRayTracingDevice();

	auto RootSignature = Shader->pRootSignature.get();

	D3D12_GPU_VIRTUAL_ADDRESS   LocalCBVs[MAX_CBS];
	D3D12_CPU_DESCRIPTOR_HANDLE LocalSRVs[MAX_SRVS];
	D3D12_CPU_DESCRIPTOR_HANDLE LocalUAVs[MAX_UAVS];
	D3D12_CPU_DESCRIPTOR_HANDLE LocalSamplers[MAX_SAMPLERS];

	uint64 BoundSRVMask = 0;
	uint64 BoundCBVMask = 0;
	uint64 BoundUAVMask = 0;
	uint64 BoundSamplerMask = 0;

	for (uint32 SRVIndex = 0; SRVIndex < InNumTextures; ++SRVIndex)
	{
		auto Resource = Textures[SRVIndex];
		if (Resource)
		{
			auto Texture =dynamic_cast<D3D12Texture*>(Resource);
			LocalSRVs[SRVIndex] = Texture->RetriveShaderResourceView(0,Resource->GetArraySize(),0,Resource->GetNumMipMaps())->GetHandle();
			BoundSRVMask |= 1ull << SRVIndex;
		}
	}

	for (uint32 SRVIndex = 0; SRVIndex < InNumSRVs; ++SRVIndex)
	{
		auto Resource = SRVs[SRVIndex];
		if (Resource)
		{
			auto SRV = static_cast<D3D12ShaderResourceView*>(Resource);
			LocalSRVs[SRVIndex] = SRV->GetHandle();
			BoundSRVMask |= 1ull << SRVIndex;
		}
	}

	for (uint32 CBVIndex = 0; CBVIndex < InNumUniformBuffers; ++CBVIndex)
	{
		auto Resource = UniformBuffers[CBVIndex];
		if (Resource)
		{
			auto CBV = static_cast<D3D12GraphicsBuffer*>(Resource);
			LocalCBVs[CBVIndex] = CBV->Resource()->GetGPUVirtualAddress();
			BoundCBVMask |= 1ull << CBVIndex;
		}
	}

	//don't support SamplerState
	for (uint32 SamplerIndex = 0; SamplerIndex < InNumSamplers; ++SamplerIndex)
	{
		auto Resource = Samplers[SamplerIndex];
		if (Resource)
		{
			LocalSamplers[SamplerIndex] = {};
			BoundSamplerMask |= 1ull << SamplerIndex;
		}
	}

	for (uint32 UAVIndex = 0; UAVIndex < InNumUAVs; ++UAVIndex)
	{
		auto Resource = UAVs[UAVIndex];
		if (Resource)
		{
			auto UAV = static_cast<D3D12UnorderedAccessView*>(Resource);
			LocalUAVs[UAVIndex] = UAV->GetHandle();
			BoundUAVMask |= 1ull << UAVIndex;
		}
	}


	// Bind loose parameters
	if (InLooseParameterData)
	{
		const uint32 CBVIndex = 0; // Global uniform buffer is always assumed to be in slot 0
		LocalCBVs[CBVIndex] = Binder.CreateTransientConstantBuffer(InLooseParameterData, InLooseParameterDataSize);
		BoundCBVMask |= 1ull << CBVIndex;
	}

	// Validate that all resources required by the shader are set

	auto IsCompleteBinding = [](uint32 ExpectedCount, uint64 BoundMask)
	{
		if (ExpectedCount > 64) return false; // Bound resource mask can't be represented by uint64

		// All bits of the mask [0..ExpectedCount) are expected to be set
		uint64 ExpectedMask = ExpectedCount == 64 ? ~0ull : ((1ull << ExpectedCount) - 1);
		return (ExpectedMask & BoundMask) == ExpectedMask;
	};
	lconstraint(IsCompleteBinding(Shader->ResourceCounts.NumSRVs, BoundSRVMask));
	lconstraint(IsCompleteBinding(Shader->ResourceCounts.NumUAVs, BoundUAVMask));
	lconstraint(IsCompleteBinding(Shader->ResourceCounts.NumCBs, BoundCBVMask));
	lconstraint(IsCompleteBinding(Shader->ResourceCounts.NumSamplers, BoundSamplerMask));

	const uint32 NumSRVs = Shader->ResourceCounts.NumSRVs;
	if (NumSRVs)
	{
		const uint32 DescriptorTableBaseIndex = DescriptorCache.GetDescriptorTableBaseIndex(LocalSRVs, NumSRVs, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

		const uint32 BindSlot = RootSignature->SRVRDTBindSlot(ShaderType::ComputeShader);
		lconstraint(BindSlot != 0xFF);

		const D3D12_GPU_DESCRIPTOR_HANDLE ResourceDescriptorTableBaseGPU = DescriptorCache.ViewHeap.GetDescriptorGPU(DescriptorTableBaseIndex);
		Binder.SetRootDescriptorTable(BindSlot, ResourceDescriptorTableBaseGPU);
	}

	const uint32 NumUAVs = Shader->ResourceCounts.NumUAVs;
	if (NumUAVs)
	{
		const uint32 DescriptorTableBaseIndex = DescriptorCache.GetDescriptorTableBaseIndex(LocalUAVs, NumUAVs, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

		const uint32 BindSlot = RootSignature->UAVRDTBindSlot(ShaderType::ComputeShader);
		lconstraint(BindSlot != 0xFF);

		const D3D12_GPU_DESCRIPTOR_HANDLE ResourceDescriptorTableBaseGPU = DescriptorCache.ViewHeap.GetDescriptorGPU(DescriptorTableBaseIndex);
		Binder.SetRootDescriptorTable(BindSlot, ResourceDescriptorTableBaseGPU);
	}

	if (Shader->ResourceCounts.NumCBs)
	{
		LAssert(RootSignature->CBVRDTBindSlot(ShaderType::ComputeShader) == 0xFF,"Root CBV descriptor tables are not implemented for ray tracing shaders.");

		const uint32 BindSlot = RootSignature->CBVRDBaseBindSlot(ShaderType::ComputeShader);
		lconstraint(BindSlot != 0xFF);

		for (uint32 i = 0; i < Shader->ResourceCounts.NumCBs; ++i)
		{
			const uint64 SlotMask = (1ull << i);
			D3D12_GPU_VIRTUAL_ADDRESS BufferAddress = (BoundCBVMask & SlotMask) ? LocalCBVs[i] : 0;
			Binder.SetRootCBV(BindSlot, i, BufferAddress);
		}
	}

	// Bind samplers

	const uint32 NumSamplers = Shader->ResourceCounts.NumSamplers;
	if (NumSamplers)
	{
		const uint32 DescriptorTableBaseIndex = DescriptorCache.GetDescriptorTableBaseIndex(LocalSamplers, NumSamplers, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);

		const uint32 BindSlot = RootSignature->SamplerRDTBindSlot(ShaderType::ComputeShader);
		lconstraint(BindSlot != 0xFF);

		const D3D12_GPU_DESCRIPTOR_HANDLE ResourceDescriptorTableBaseGPU = DescriptorCache.SamplerHeap.GetDescriptorGPU(DescriptorTableBaseIndex);
		Binder.SetRootDescriptorTable(BindSlot, ResourceDescriptorTableBaseGPU);
	}
}

template <typename ResourceBinderType>
static void SetRayTracingShaderResources(
	D3D12RayContext& CommandContext,
	const platform_ex::Windows::D3D12::RayTracingShader* Shader,
	const RayTracingShaderBindings& ResourceBindings,
	RayTracingDescriptorCache& DescriptorCache,
	ResourceBinderType& Binder)
{
	SetRayTracingShaderResources(CommandContext, Shader,
		leo::size(ResourceBindings.Textures), ResourceBindings.Textures,
		leo::size(ResourceBindings.SRVs), ResourceBindings.SRVs,
		leo::size(ResourceBindings.UniformBuffers), ResourceBindings.UniformBuffers,
		leo::size(ResourceBindings.Samplers), ResourceBindings.Samplers,
		leo::size(ResourceBindings.UAVs), ResourceBindings.UAVs,
		0,nullptr,
		DescriptorCache,Binder
		);
}

void DispatchRays(D3D12RayContext* CommandContext, const RayTracingShaderBindings& GlobalBindings, const D3D12RayTracingPipelineState* Pipeline,
	uint32 RayGenShaderIndex, RayTracingShaderTable* OptShaderTable, const D3D12_DISPATCH_RAYS_DESC& DispatchDesc)
{
	CommandContext->RayTracingCommandList()->SetComputeRootSignature(Pipeline->GetRootSignature());

	auto RayGenShader = Pipeline->RayGenShaders.Shaders[RayGenShaderIndex].get();

	if (OptShaderTable && OptShaderTable->DescriptorCache)
	{
		//TODO
	}
	else
	{
		RayTracingDescriptorCache TransientDescriptorCache(&CommandContext->GetDevice());

		TransientDescriptorCache.Init(MAX_SRVS + MAX_UAVS, MAX_SAMPLERS);
		TransientDescriptorCache.SetDescriptorHeaps(*CommandContext);

		FD3D12RayTracingGlobalResourceBinder ResourceBinder(*CommandContext);
		SetRayTracingShaderResources(*CommandContext, RayGenShader, GlobalBindings, TransientDescriptorCache, ResourceBinder);
	}

	auto StateObject = Pipeline->StateObject.Get();

	auto RayTracingCommandList = CommandContext->RayTracingCommandList();

	RayTracingCommandList->SetPipelineState1(StateObject);
	RayTracingCommandList->DispatchRays(&DispatchDesc);
}

