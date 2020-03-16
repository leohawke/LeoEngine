#pragma once

#include "d3d12_dxgi.h"
#include <LBase/string.hpp>
#include <LBase/span.hpp>
#include "GraphicsBuffer.hpp"
#include "RayTracingShader.h"
#include "RootSignature.h"
#include "../RayTracingDefinitions.h"
#include "../IGPUResourceView.h"
#include <LBase/type_traits.hpp>

//pre decl
namespace platform_ex::Windows::D3D12 {
	class RayTracingPipelineState;
	class Device;
	class Context;
	class RayDevice;
	class RayContext;
}

using namespace leo::inttype;
using namespace platform_ex;

template< typename t_A, typename t_B >
inline t_A RoundUpToNextMultiple(const t_A& a, const t_B& b)
{
	return ((a - 1) / b + 1) * b;
}



struct HitGroupSystemRootConstants
{
	// Config is a bitfield:
	// uint IndexStride  : 8; // Can be just 1 bit to indicate 16 or 32 bit indices
	// uint VertexStride : 8; // Can be just 2 bits to indicate float3, float2 or half2 format
	// uint Unused       : 16;
	unsigned int Config;

	// Offset into HitGroupSystemIndexBuffer
	unsigned int IndexBufferOffsetInBytes;

	// User-provided constant assigned to the hit group
	unsigned int UserData;

	// Padding to ensure that root parameters are properly aligned to 8-byte boundary
	unsigned int Unused;

	// Helper functions

	unsigned int GetIndexStride()
	{
		return Config & 0xFF;
	}

	unsigned int GetVertexStride()
	{
		return (Config >> 8) & 0xFF;
	}

	void SetVertexAndIndexStride(unsigned int Vertex, unsigned int Index)
	{
		Config = (Index & 0xFF) | ((Vertex & 0xFF) << 8);
	}
};

#define RAY_TRACING_SYSTEM_INDEXBUFFER_REGISTER  0
#define RAY_TRACING_SYSTEM_VERTEXBUFFER_REGISTER 1
#define RAY_TRACING_SYSTEM_ROOTCONSTANT_REGISTER 0

struct ShaderIdentifier
{
	uint64 Data[4] = { ~0ull, ~0ull, ~0ull, ~0ull };

	bool operator == (const ShaderIdentifier& Other) const
	{
		return Data[0] == Other.Data[0]
			&& Data[1] == Other.Data[1]
			&& Data[2] == Other.Data[2]
			&& Data[3] == Other.Data[3];
	}

	bool operator != (const ShaderIdentifier& Other) const
	{
		return !(*this == Other);
	}

	bool IsValid() const
	{
		return *this != ShaderIdentifier();
	}

	// No shader is executed if a shader binding table record with null identifier is encountered.
	void SetNull()
	{
		Data[3] = Data[2] = Data[1] = Data[0] = 0ull;
	}

	void SetData(const void* InData)
	{
		std::memcpy(Data, InData, sizeof(Data));
	}
};

static_assert(sizeof(ShaderIdentifier) == D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES, "Unexpected shader identifier size");

struct DXILLibrary
{
	DXILLibrary(const void* Bytecode, leo::uint32 BytecodeLength, const LPCWSTR* InEntryNames, const LPCWSTR* InExportNames, leo::uint32 NumEntryNames)
	{
		EntryNames.resize(NumEntryNames);
		ExportNames.resize(NumEntryNames);
		ExportDesc.resize(NumEntryNames);

		for (uint32 EntryIndex = 0; EntryIndex < NumEntryNames; ++EntryIndex)
		{
			EntryNames[EntryIndex] = InEntryNames[EntryIndex];
			ExportNames[EntryIndex] = InExportNames[EntryIndex];

			ExportDesc[EntryIndex].ExportToRename = EntryNames[EntryIndex].c_str();
			ExportDesc[EntryIndex].Flags = D3D12_EXPORT_FLAG_NONE;
			ExportDesc[EntryIndex].Name = ExportNames[EntryIndex].c_str();
		}

		Desc.DXILLibrary.pShaderBytecode = Bytecode;
		Desc.DXILLibrary.BytecodeLength = BytecodeLength;
		Desc.NumExports =static_cast<UINT>(ExportDesc.size());
		Desc.pExports = ExportDesc.data();
	}

	D3D12_STATE_SUBOBJECT GetSubobject() const
	{
		D3D12_STATE_SUBOBJECT Subobject = {};
		Subobject.Type = D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY;
		Subobject.pDesc = &Desc;
		return Subobject;
	}

	leo::vector<D3D12_EXPORT_DESC> ExportDesc;
	leo::vector<leo::wstring> EntryNames;
	leo::vector<leo::wstring> ExportNames;
	D3D12_DXIL_LIBRARY_DESC Desc = {};
};

COMPtr<ID3D12StateObject> CreateRayTracingStateObject(
	ID3D12Device5* RayTracingDevice,
	const leo::span<const DXILLibrary*>& ShaderLibraries,
	const leo::span<LPCWSTR>& Exports,
	uint32 MaxPayloadSizeInBytes,
	const leo::span<const D3D12_HIT_GROUP_DESC>& HitGroups,
	const ID3D12RootSignature* GlobalRootSignature,
	const leo::span<ID3D12RootSignature*>& LocalRootSignatures,
	const leo::span<uint32>& LocalRootSignatureAssociations, // indices into LocalRootSignatures, one per export (may be empty, which assumes single root signature used for everything)
	const leo::span<D3D12_EXISTING_COLLECTION_DESC>& ExistingCollections,
	D3D12_STATE_OBJECT_TYPE StateObjectType // Full RTPSO or a Collection
);


using D3D12Context = Windows::D3D12::Context;
using D3D12Device = Windows::D3D12::Device;
using D3D12RayDevice = Windows::D3D12::RayDevice;
using D3D12RayContext = Windows::D3D12::RayContext;
using D3D12RayTracingPipelineState = platform_ex::Windows::D3D12::RayTracingPipelineState;
using D3D12RayTracingShader = Windows::D3D12::RayTracingShader;

class RayDeviceChild
{
public:
	RayDeviceChild(D3D12RayDevice* InDevice)
		:Device(InDevice)
	{}
protected:
	D3D12RayDevice* Device;
};


// ad-hoc heaps
// This would allow ray tracing code to sub-allocate heap blocks from the same global heap.
class RayTracingDescriptorHeapCache : public RayDeviceChild
{
public:
	struct Entry
	{
		ID3D12DescriptorHeap* Heap = nullptr;
		uint64 FenceValue = 0;
		uint32 NumDescriptors = 0;
		D3D12_DESCRIPTOR_HEAP_TYPE Type = D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES;
	};

	RayTracingDescriptorHeapCache(D3D12RayDevice* InDevice)
		:RayDeviceChild(InDevice)
	{
	}

	~RayTracingDescriptorHeapCache();

	void ReleaseHeap(Entry& Entry);

	Entry AllocateHeap(D3D12_DESCRIPTOR_HEAP_TYPE Type, uint32 NumDescriptors);

	void ReleaseStaleEntries(uint32 MaxAge, uint64 CompletedFenceValue);

private:
	std::mutex CriticalSection;
	std::vector<Entry> Entries;
	uint32 AllocatedEntries = 0;
};

struct RayTracingDescriptorHeap :RayDeviceChild
{
	RayTracingDescriptorHeap(D3D12RayDevice* Device)
		:RayDeviceChild(Device)
	{
	}

	~RayTracingDescriptorHeap();

	void Init(uint32 InMaxNumDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE Type);
	bool CanAllocate(uint32 InNumDescriptors) const;
	uint32 Allocate(uint32 InNumDescriptors);

	D3D12_CPU_DESCRIPTOR_HANDLE GetDescriptorCPU(uint32 Index) const
	{
		D3D12_CPU_DESCRIPTOR_HANDLE Result = { CPUBase.ptr + Index * DescriptorSize };
		return Result;
	}
	D3D12_GPU_DESCRIPTOR_HANDLE GetDescriptorGPU(uint32 Index) const
	{
		D3D12_GPU_DESCRIPTOR_HANDLE Result = { GPUBase.ptr + Index * DescriptorSize };
		return Result;
	}

	void UpdateSyncPoint();

	ID3D12DescriptorHeap* D3D12Heap = nullptr;
	uint32 MaxNumDescriptors = 0;
	uint32 NumAllocatedDescriptors = 0;

	uint32 DescriptorSize = 0;
	D3D12_CPU_DESCRIPTOR_HANDLE CPUBase = {};
	D3D12_GPU_DESCRIPTOR_HANDLE GPUBase = {};

	RayTracingDescriptorHeapCache::Entry HeapCacheEntry;
};

class RayTracingDescriptorCache : public RayDeviceChild
{
public:
	RayTracingDescriptorCache(D3D12RayDevice* Device)
		:RayDeviceChild(Device),
		ViewHeap(Device),
		SamplerHeap(Device)
	{}

	void Init(uint32 NumViewDescriptors, uint32 NumTextureSampleDescriptors)
	{
		ViewHeap.Init(NumViewDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		SamplerHeap.Init(NumTextureSampleDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
	}

	void UpdateSyncPoint()
	{
		ViewHeap.UpdateSyncPoint();
		SamplerHeap.UpdateSyncPoint();
	}

	void SetDescriptorHeaps(D3D12RayContext& Context);

	uint32 GetDescriptorTableBaseIndex(const D3D12_CPU_DESCRIPTOR_HANDLE* Descriptors, uint32 NumDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE Type);
public:
	RayTracingDescriptorHeap ViewHeap;
	RayTracingDescriptorHeap SamplerHeap;
private:
	std::map<uint64, uint32> ViewDescriptorTableCache;
	std::map<uint64, uint32> TextureSampleDescriptorTableCache;
};

class RayTracingShaderTable
{
private:
	void WriteData(uint32 WriteOffset, const void* InData, uint32 InDataSize)
	{
		std::memcpy(reinterpret_cast<std::byte*>(Data.data()) + WriteOffset, InData, InDataSize);

		bIsDirty = true;
	}

	void WriteLocalShaderRecord(uint32 RecordIndex, uint32 OffsetWithinRecord, const void* InData, uint32 InDataSize)
	{
		LAssert(OffsetWithinRecord % 4 == 0, "SBT record parameters must be written on DWORD-aligned boundary");
		LAssert(InDataSize % 4 == 0, "SBT record parameters must be DWORD-aligned");
		LAssert(OffsetWithinRecord + InDataSize <= LocalRecordSizeUnaligned, "SBT record write request is out of bounds");
		LAssert(RecordIndex < NumLocalRecords, "SBT local record write request is out of bounds");

		const uint32 WriteOffset = LocalShaderTableOffset + LocalRecordStride * RecordIndex + OffsetWithinRecord;

		WriteData(WriteOffset, InData, InDataSize);
	}
public:
	struct Initializer
	{
		uint32 NumRayGenShaders = 0;
		uint32 NumMissShaders = 0;
		uint32 NumHitRecords = 0;
		uint32 NumCallableRecords = 0;
		uint32 LocalRootDataSize = 0;
		uint32 MaxViewDescriptorsPerRecord = 0;
	};

	void Init(const Initializer& initializer,D3D12RayDevice* Device)
	{
		LAssert(initializer.LocalRootDataSize <= 4096, "The maximum size of a local root signature is 4KB."); // as per section 4.22.1 of DXR spec v1.0
		LAssert(initializer.NumRayGenShaders >= 1, "All shader tables must contain at least one raygen shader.");

		LocalRecordSizeUnaligned = ShaderIdentifierSize + initializer.LocalRootDataSize;
		LocalRecordStride = RoundUpToNextMultiple(LocalRecordSizeUnaligned, D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT);

		// Custom descriptor cache is only required when local resources may be bound.
		// If only global resources are used, then transient descriptor cache can be used.
		const bool bNeedsDescriptorCache = (initializer.NumHitRecords + initializer.NumCallableRecords) * initializer.LocalRootDataSize != 0;
		if (bNeedsDescriptorCache)
		{
			// RT descriptors aren't sub-allocated from the global view descriptor heap.
			const uint32 MinNumViewDescriptors = 1024;

			auto NumViewDescriptors =  std::clamp<uint32>(initializer.NumHitRecords * initializer.MaxViewDescriptorsPerRecord, MinNumViewDescriptors, D3D12_MAX_SHADER_VISIBLE_DESCRIPTOR_HEAP_SIZE_TIER_1);
			
			const uint32 NumTextureSampleDescriptors = D3D12_MAX_SHADER_VISIBLE_SAMPLER_HEAP_SIZE;

			DescriptorCache = new RayTracingDescriptorCache(Device);
			DescriptorCache->Init(NumViewDescriptors, NumTextureSampleDescriptors);
		}

		//how about descriptor

		NumRayGenShaders = initializer.NumRayGenShaders;
		NumMissShaders = initializer.NumMissShaders;
		NumHitRecords = initializer.NumHitRecords;
		NumCallableRecords = initializer.NumCallableRecords;


		uint32 TotalDataSize = 0;

		RayGenShaderTableOffset = TotalDataSize;
		TotalDataSize += NumRayGenShaders * RayGenRecordStride;
		TotalDataSize = RoundUpToNextMultiple(TotalDataSize, D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT);

		MissShaderTableOffset = TotalDataSize;
		TotalDataSize += NumMissShaders * MissRecordStride;
		TotalDataSize = RoundUpToNextMultiple(TotalDataSize, D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT);

		DefaultHitGroupShaderTableOffset = TotalDataSize;
		TotalDataSize += ShaderIdentifierSize;
		TotalDataSize = RoundUpToNextMultiple(TotalDataSize, D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT);

		HitGroupShaderTableOffset = TotalDataSize;
		TotalDataSize += initializer.NumHitRecords * LocalRecordStride;
		TotalDataSize = RoundUpToNextMultiple(TotalDataSize, D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT);

		CallableShaderTableOffset = TotalDataSize;
		TotalDataSize += initializer.NumCallableRecords * LocalRecordStride;
		TotalDataSize = RoundUpToNextMultiple(TotalDataSize, D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT);

		// Hit groups and callable shaders are stored in a consecutive memory block and are accessed using common local record indices.
		LocalShaderTableOffset = HitGroupShaderTableOffset;
		CallableShaderRecordIndexOffset = (CallableShaderTableOffset - LocalShaderTableOffset) / LocalRecordStride;
		NumLocalRecords = (TotalDataSize - LocalShaderTableOffset) / LocalRecordStride;

		Data.resize(TotalDataSize);
	}

	void SetRayGenIdentifier(uint32 RecordIndex, const ShaderIdentifier& ShaderIdentifier)
	{
		const uint32 WriteOffset = RayGenShaderTableOffset + RecordIndex * RayGenRecordStride;
		WriteData(WriteOffset, ShaderIdentifier.Data, ShaderIdentifierSize);
	}

	void SetMissIdentifier(uint32 RecordIndex, const ShaderIdentifier& ShaderIdentifier)
	{
		const uint32 WriteOffset = MissShaderTableOffset + RecordIndex * MissRecordStride;
		WriteData(WriteOffset, ShaderIdentifier.Data, ShaderIdentifierSize);
	}

	void SetDefaultHitGroupIdentifier(const ShaderIdentifier& ShaderIdentifier)
	{
		const uint32 WriteOffset = DefaultHitGroupShaderTableOffset;
		WriteData(WriteOffset, ShaderIdentifier.Data, ShaderIdentifierSize);
	}

	void SetRayGenIdentifiers(const leo::span<const ShaderIdentifier>& Identifiers)
	{
		lconstraint(Identifiers.size() == NumRayGenShaders);
		for (int32 Index = 0; Index < Identifiers.size(); ++Index)
		{
			SetRayGenIdentifier(Index, Identifiers[Index]);
		}
	}

	void SetMissIdentifiers(const leo::span<const ShaderIdentifier>& Identifiers)
	{
		lconstraint(Identifiers.size() == NumMissShaders);
		for (int32 Index = 0; Index < Identifiers.size(); ++Index)
		{
			SetMissIdentifier(Index, Identifiers[Index]);
		}
	}


	void UploadToGPU(Windows::D3D12::Device* Device);

	D3D12_GPU_VIRTUAL_ADDRESS GetShaderTableAddress() const
	{
		LAssert(!bIsDirty, "Shader table update is pending, therefore GPU address is not available. Use UploadToGPU() to upload data and acquire a valid GPU buffer address.");
		return Buffer->Resource()->GetGPUVirtualAddress();
	}


	D3D12_DISPATCH_RAYS_DESC GetDispatchRaysDesc(uint32 RayGenShaderIndex, uint32 MissShaderBaseIndex, bool bAllowHitGroupIndexing) const
	{
		D3D12_GPU_VIRTUAL_ADDRESS ShaderTableAddress = GetShaderTableAddress();

		D3D12_DISPATCH_RAYS_DESC Desc = {};

		Desc.RayGenerationShaderRecord.StartAddress = ShaderTableAddress + RayGenShaderTableOffset + RayGenShaderIndex * RayGenRecordStride;
		Desc.RayGenerationShaderRecord.SizeInBytes = RayGenRecordStride;

		Desc.MissShaderTable.StartAddress = ShaderTableAddress + MissShaderTableOffset + MissShaderBaseIndex * MissRecordStride;
		Desc.MissShaderTable.StrideInBytes = MissRecordStride;
		Desc.MissShaderTable.SizeInBytes = MissRecordStride;

		if (NumCallableRecords)
		{
			Desc.CallableShaderTable.StartAddress = ShaderTableAddress + CallableShaderTableOffset;
			Desc.CallableShaderTable.StrideInBytes = LocalRecordStride;
			Desc.CallableShaderTable.SizeInBytes = NumCallableRecords * LocalRecordStride;
		}

		if (bAllowHitGroupIndexing)
		{
			Desc.HitGroupTable.StartAddress = ShaderTableAddress + HitGroupShaderTableOffset;
			Desc.HitGroupTable.StrideInBytes = LocalRecordStride;
			Desc.HitGroupTable.SizeInBytes = NumHitRecords * LocalRecordStride;
		}
		else
		{
			Desc.HitGroupTable.StartAddress = ShaderTableAddress + DefaultHitGroupShaderTableOffset;
			Desc.HitGroupTable.StrideInBytes = 0; // Zero stride effectively disables SBT indexing
			Desc.HitGroupTable.SizeInBytes = D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT; // Minimal table with only one record
		}

		return Desc;
	}

	static constexpr uint32 ShaderIdentifierSize = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
	uint32 NumHitRecords = 0;
	uint32 NumRayGenShaders = 0;
	uint32 NumMissShaders = 0;
	uint32 NumCallableRecords = 0;
	uint32 NumLocalRecords = 0;

	uint32 RayGenShaderTableOffset = 0;
	uint32 MissShaderTableOffset = 0;
	uint32 DefaultHitGroupShaderTableOffset = 0;
	uint32 HitGroupShaderTableOffset = 0;
	uint32 CallableShaderTableOffset = 0;
	uint32 LocalShaderTableOffset = 0;
	uint32 CallableShaderRecordIndexOffset = 0;

	// Note: TABLE_BYTE_ALIGNMENT is used instead of RECORD_BYTE_ALIGNMENT to allow arbitrary switching 
	// between multiple RayGen and Miss shaders within the same underlying table.
	static constexpr uint32 RayGenRecordStride = D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT;
	static constexpr uint32 MissRecordStride = D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT;

	uint32 LocalRecordSizeUnaligned = 0; // size of the shader identifier + local root parameters, not aligned to SHADER_RECORD_BYTE_ALIGNMENT (used for out-of-bounds access lconstraints)
	uint32 LocalRecordStride = 0; // size of shader identifier + local root parameters, aligned to SHADER_RECORD_BYTE_ALIGNMENT (same for hit groups and callable shaders)

	static constexpr uint32 RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT = D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT;
	using aligned_byte =typename std::aligned_storage<1, 64>::type;
	std::vector<aligned_byte> Data;

	shared_ptr < Windows::D3D12::GraphicsBuffer> Buffer;

	RayTracingDescriptorCache* DescriptorCache = nullptr;

	bool bIsDirty = true;
};



struct RayTracingShaderLibrary
{
	leo::vector <leo::shared_ptr<Windows::D3D12::RayTracingShader>> Shaders;
	leo::vector<ShaderIdentifier> Identifiers;

	void Reserve(leo::uint32 Num)
	{
		Shaders.reserve(Num);
		Identifiers.reserve(Num);
	}
};

class RayTracingPipelineCache
{
public:
	RayTracingPipelineCache(ID3D12Device5* Device);

	enum class CollectionType
	{
		RayGen,
		Miss,
		HitGroup,
		Callable,
	};

	static const char* GetCollectionTypeName(CollectionType Type)
	{
		switch (Type)
		{
		case CollectionType::RayGen:
			return "RayGen";
		case CollectionType::Miss:
			return "Miss";
		case CollectionType::HitGroup:
			return "HitGroup";
		case CollectionType::Callable:
			return "Callable";
		default:
			return "";
		}
	}

	struct Key
	{
		union
		{
			uint64 ShaderHash;
			intptr_t ShaderPoint;
		};
		uint32 MaxPayloadSizeInBytes = 0;
		ID3D12RootSignature* GlobalRootSignature = nullptr;
		ID3D12RootSignature* LocalRootSignature = nullptr;

		bool operator == (const Key& Other) const
		{
			return ShaderHash == Other.ShaderHash
				&& MaxPayloadSizeInBytes == Other.MaxPayloadSizeInBytes
				&& GlobalRootSignature == Other.GlobalRootSignature
				&& LocalRootSignature == Other.LocalRootSignature;
		}
	};

	struct KeyHash
	{
		std::size_t operator()(const Key& key) const
		{
			return key.ShaderHash;
		}
	};

	struct Entry
	{
		leo::observer_ptr<Windows::D3D12::RayTracingShader> Shader;
		COMPtr<ID3D12StateObject> StateObject;
		std::vector<leo::Text::String> ExportNames;
		std::vector<D3D12_EXPORT_DESC> ExportDescs;

		const wchar_t* GetPrimaryExportNameChars()
		{
			LAssert(ExportNames.size() != 0,"This ray tracing shader collection does not export any symbols.");
			return reinterpret_cast<const wchar_t*>(ExportNames[0].c_str());
		}
	};

	Entry* GetOrCompileShader(ID3D12Device5* RayTracingDevice,
		D3D12RayTracingShader* Shader,
		ID3D12RootSignature* GlobalRootSignature,
		uint32 MaxPayloadSizeInBytes,
		CollectionType CollectionType);

private:
	std::mutex CriticalSection;
	std::unordered_map<Key, Entry*, KeyHash> Cache;
	Windows::D3D12::RootSignature DefaultLocalRootSignature;
};

struct RayTracingShaderBindings
{
	platform::Render::Texture* Textures[32] = {};
	platform::Render::ShaderResourceView* SRVs[32] = {};
	platform::Render::GraphicsBuffer* UniformBuffers[32] = {};
	platform::Render::TextureSampleDesc* Samplers[32] = {};
	platform::Render::UnorderedAccessView* UAVs[8] = {};
};

void DispatchRays(D3D12RayContext* CommandContext,const RayTracingShaderBindings& GlobalBindings, const D3D12RayTracingPipelineState* Pipeline,
	uint32 RayGenShaderIndex, RayTracingShaderTable* OptShaderTable, const D3D12_DISPATCH_RAYS_DESC& DispatchDesc);

