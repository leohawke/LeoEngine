#pragma once

#include "d3d12_dxgi.h"
#include <LBase/string.hpp>
#include <LBase/span.hpp>
#include "GraphicsBuffer.hpp"
#include "RayTracingShader.h"
#include <LBase/type_traits.hpp>

using namespace leo::inttype;
using namespace platform_ex;

template< typename t_A, typename t_B >
inline t_A RoundUpToNextMultiple(const t_A& a, const t_B& b)
{
	return ((a - 1) / b + 1) * b;
}

#define RAY_TRACING_REGISTER_SPACE_LOCAL  0 // default register space for hit group (closest hit, any hit, intersection) shader resources
#define RAY_TRACING_REGISTER_SPACE_GLOBAL 1 // register space for ray generation and miss shaders 
#define RAY_TRACING_REGISTER_SPACE_SYSTEM 2 // register space for "system" parameters (index buffer, vertex buffer, fetch parameters)

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

	void Init(const Initializer& initializer)
	{
		LAssert(initializer.LocalRootDataSize <= 4096, "The maximum size of a local root signature is 4KB."); // as per section 4.22.1 of DXR spec v1.0
		LAssert(initializer.NumRayGenShaders >= 1, "All shader tables must contain at least one raygen shader.");

		LocalRecordSizeUnaligned = ShaderIdentifierSize + initializer.LocalRootDataSize;
		LocalRecordStride = RoundUpToNextMultiple(LocalRecordSizeUnaligned, D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT);

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
	enum class CollectionType
	{
		RayGen,
		Miss,
		HitGroup,
		Callable,
	};
};

