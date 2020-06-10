#pragma once

#include <LBase/ldef.h>
#include <Engine/Core/Hash/CityHash.h>

#include "../ShaderCore.h"

#include "d3d12_dxgi.h"
#include "Common.h"
#include "Fence.h"
#include <unordered_set>
#include <mutex>
#include <queue>

namespace platform_ex::Windows::D3D12 {
	class CommandContext;
	class DescriptorCache;
	class RenderTargetView;
	class DepthStencilView;

	struct VertexBufferCache;
	struct UnorderedAccessViewCache;
	struct ShaderResourceViewCache;
	struct SamplerStateCache;
	struct ConstantBufferCache;

	class RootSignature;
	class ResourceHolder;

	using platform::Render::ShaderType;

	// Like a std::unordered_map<KeyType, ValueType>
	// Faster lookup performance, but possibly has false negatives
	template<typename KeyType, typename ValueType>
	class ConservativeMap
	{
	public:
		ConservativeMap(uint32 Size)
		{
			Table.resize(Size);

			Reset();
		}

		void Add(const KeyType& Key, const ValueType& Value)
		{
			uint32 Index = GetIndex(Key);

			Entry& Pair = Table[Index];

			Pair.Valid = true;
			Pair.Key = Key;
			Pair.Value = Value;
		}

		ValueType* Find(const KeyType& Key)
		{
			uint32 Index = GetIndex(Key);

			Entry& Pair = Table[Index];

			if (Pair.Valid &&
				(Pair.Key == Key))
			{
				return &Pair.Value;
			}
			else
			{
				return nullptr;
			}
		}

		void Reset()
		{
			for (int32 i = 0; i < Table.size(); i++)
			{
				Table[i].Valid = false;
			}
		}
	private:
		uint32 GetIndex(const KeyType& Key)
		{
			uint32 Hash = GetTypeHash(Key);

			return Hash % static_cast<uint32>(Table.size());
		}

		struct Entry
		{
			bool Valid = false;
			KeyType Key;
			ValueType Value;
		};

		std::vector<Entry> Table;
	};

	struct SamplerArrayDesc
	{
		uint32 Count;
		uint16 SamplerID[16];

		bool operator==(const SamplerArrayDesc& rhs) const = default;
	};

	uint32 GetTypeHash(const SamplerArrayDesc& Key);

	using SamplerMap = ConservativeMap<SamplerArrayDesc, D3D12_GPU_DESCRIPTOR_HANDLE>;

	template< uint32 CPUTableSize>
	struct UniqueDescriptorTable
	{
		UniqueDescriptorTable() : GPUHandle({}) {};
		UniqueDescriptorTable(SamplerArrayDesc KeyIn, CD3DX12_CPU_DESCRIPTOR_HANDLE* Table) : GPUHandle({})
		{
			std::memcpy(&Key, &KeyIn, sizeof(Key));//Memcpy to avoid alignement issues
			std::memcpy(CPUTable, Table, Key.Count * sizeof(CD3DX12_CPU_DESCRIPTOR_HANDLE));
		}

		uint32 GetTypeHash(const UniqueDescriptorTable& Table)
		{
			return CityHash64((void*)Table.Key.SamplerID, Table.Key.Count * sizeof(Table.Key.SamplerID[0]));
		}

		SamplerArrayDesc Key;
		CD3DX12_CPU_DESCRIPTOR_HANDLE CPUTable[MAX_SAMPLERS];

		// This will point to the table start in the global heap
		D3D12_GPU_DESCRIPTOR_HANDLE GPUHandle;

		bool operator=(const UniqueDescriptorTable& rhs) const
		{
			return Key == rhs.Key;
		}
	};

	template<typename UniqueSamplerTable>
	struct UniqueDescriptorTableHasher
	{
		size_t operator()(const UniqueSamplerTable& key) const noexcept {
			return key.GetTypeHash();
		}
	};

	using UniqueSamplerTable = UniqueDescriptorTable<MAX_SAMPLERS>;

	using SamplerSet = std::unordered_set<UniqueSamplerTable, UniqueDescriptorTableHasher<UniqueSamplerTable>>;

	class OnlineHeap : public DeviceChild,public SingleNodeGPUObject
	{
	public:
		OnlineHeap(D3D12Device* Device, bool CanLoopAround, DescriptorCache* _Parent = nullptr, GPUMaskType Node = 0);
		virtual ~OnlineHeap() { }

		D3D12_CPU_DESCRIPTOR_HANDLE GetCPUSlotHandle(uint32 Slot) const { return{ CPUBase.ptr + Slot * DescriptorSize }; }
		D3D12_GPU_DESCRIPTOR_HANDLE GetGPUSlotHandle(uint32 Slot) const { return{ GPUBase.ptr + Slot * DescriptorSize }; }

		inline const uint32 GetDescriptorSize() const { return DescriptorSize; }

		const D3D12_DESCRIPTOR_HEAP_DESC& GetDesc() const { return Desc; }

		// Call this to reserve descriptor heap slots for use by the command list you are currently recording. This will wait if
		// necessary until slots are free (if they are currently in use by another command list.) If the reservation can be
		// fulfilled, the index of the first reserved slot is returned (all reserved slots are consecutive.) If not, it will 
		// throw an exception.
		bool CanReserveSlots(uint32 NumSlots);

		uint32 ReserveSlots(uint32 NumSlotsRequested);

		void SetNextSlot(uint32 NextSlot);

		ID3D12DescriptorHeap* GetHeap()
		{
			return Heap.Get();
		}

		void SetParent(DescriptorCache* InParent) { Parent = InParent; }

		// Roll over behavior depends on the heap type
		virtual bool RollOver() = 0;
		virtual void NotifyCurrentCommandList(ID3D12GraphicsCommandList& CommandListHandle);

		virtual uint32 GetTotalSize()
		{
			return Desc.NumDescriptors;
		}

		static const uint32 HeapExhaustedValue = uint32(-1);
	protected:
		DescriptorCache* Parent;

		ID3D12GraphicsCommandList* CurrentCommandList;


		// Handles for manipulation of the heap
		uint32 DescriptorSize;
		D3D12_CPU_DESCRIPTOR_HANDLE CPUBase;
		D3D12_GPU_DESCRIPTOR_HANDLE GPUBase;

		// This index indicate where the next set of descriptors should be placed *if* there's room
		uint32 NextSlotIndex;

		// Indicates the last free slot marked by the command list being finished
		uint32 FirstUsedSlot;

		// Keeping this ptr around is basically just for lifetime management
		COMPtr<ID3D12DescriptorHeap> Heap;

		// Desc contains the number of slots and allows for easy recreation
		D3D12_DESCRIPTOR_HEAP_DESC Desc;

		const bool bCanLoopAround;
	};

	class GlobalOnlineHeap : public OnlineHeap
	{
	public:
		GlobalOnlineHeap(D3D12Device* Device, GPUMaskType Node =0)
			: OnlineHeap(Device, false,nullptr,Node)
			, bUniqueDescriptorTablesAreDirty(false)
		{ }

		void Init(uint32 TotalSize, D3D12_DESCRIPTOR_HEAP_TYPE Type);

		void ToggleDescriptorTablesDirtyFlag(bool Value) { bUniqueDescriptorTablesAreDirty = Value; }
		bool DescriptorTablesDirty() { return bUniqueDescriptorTablesAreDirty; }
		SamplerSet& GetUniqueDescriptorTables() { return UniqueDescriptorTables; }
		std::mutex& GetCriticalSection() { return CriticalSection; }

		bool RollOver();
	private:
		SamplerSet UniqueDescriptorTables;
		bool bUniqueDescriptorTablesAreDirty;

		std::mutex CriticalSection;
	};

	struct OnlineHeapBlock
	{
	public:
		OnlineHeapBlock(uint32 _BaseSlot, uint32 _Size) :
			BaseSlot(_BaseSlot), Size(_Size), SizeUsed(0), bFresh(true) {};
		OnlineHeapBlock() : BaseSlot(0), Size(0), SizeUsed(0), bFresh(true) {}

		CLSyncPoint SyncPoint;
		uint32 BaseSlot;
		uint32 Size;
		uint32 SizeUsed;
		// Indicates that this has never been used in a Command List before
		bool bFresh;
	};

	class SubAllocatedOnlineHeap : public OnlineHeap
	{
	public:
		struct SubAllocationDesc
		{
			SubAllocationDesc() :ParentHeap(nullptr), BaseSlot(0), Size(0) {};
			SubAllocationDesc(GlobalOnlineHeap* _ParentHeap, uint32 _BaseSlot, uint32 _Size) :
				ParentHeap(_ParentHeap), BaseSlot(_BaseSlot), Size(_Size) {};

			GlobalOnlineHeap* ParentHeap;
			uint32 BaseSlot;
			uint32 Size;
		};

		SubAllocatedOnlineHeap(D3D12Device* Device, DescriptorCache* Parent, GPUMaskType Node = 0) :
			OnlineHeap(Device, false, Parent,Node) {};

		void Init(SubAllocationDesc _Desc);

		// Specializations
		bool RollOver();
		void NotifyCurrentCommandList(ID3D12GraphicsCommandList& CommandListHandle) override;

		virtual uint32 GetTotalSize() final override
		{
			return CurrentSubAllocation.Size;
		}
	private:

		std::queue<OnlineHeapBlock> DescriptorBlockPool;
		SubAllocationDesc SubDesc;

		OnlineHeapBlock CurrentSubAllocation;
	};

	class ThreadLocalOnlineHeap :public OnlineHeap
	{
	public:
		ThreadLocalOnlineHeap(D3D12Device* Device, DescriptorCache* _Parent, GPUMaskType Node=0)
			: OnlineHeap(Device, true, _Parent,Node)
		{ }

		bool RollOver();

		void NotifyCurrentCommandList(ID3D12GraphicsCommandList& CommandListHandle) override;

		void Init(uint32 NumDescriptors, D3D12_DESCRIPTOR_HEAP_TYPE Type);

	private:
		struct SyncPointEntry
		{
			CLSyncPoint SyncPoint;
			uint32 LastSlotInUse;

			SyncPointEntry() : LastSlotInUse(0)
			{}

			SyncPointEntry(const SyncPointEntry& InSyncPoint) : SyncPoint(InSyncPoint.SyncPoint), LastSlotInUse(InSyncPoint.LastSlotInUse)
			{}

			SyncPointEntry& operator = (const SyncPointEntry& InSyncPoint)
			{
				SyncPoint = InSyncPoint.SyncPoint;
				LastSlotInUse = InSyncPoint.LastSlotInUse;

				return *this;
			}
		};
		std::queue<SyncPointEntry> SyncPoints;

		struct PoolEntry
		{
			COMPtr<ID3D12DescriptorHeap> Heap;
			CLSyncPoint SyncPoint;

			PoolEntry()
			{}

			PoolEntry(const PoolEntry& InPoolEntry) : Heap(InPoolEntry.Heap), SyncPoint(InPoolEntry.SyncPoint)
			{}

			PoolEntry& operator = (const PoolEntry& InPoolEntry)
			{
				Heap = InPoolEntry.Heap;
				SyncPoint = InPoolEntry.SyncPoint;
				return *this;
			}
		};
		PoolEntry Entry;
		std::queue<PoolEntry> ReclaimPool;
	};

	class DescriptorCache : public DeviceChild,public SingleNodeGPUObject
	{
	protected:
		CommandContext* CmdContext;
	public:
		OnlineHeap* GetCurrentViewHeap() { return CurrentViewHeap; }
		OnlineHeap* GetCurrentSamplerHeap() { return CurrentSamplerHeap; }

		DescriptorCache(GPUMaskType Node =0);

		~DescriptorCache()
		{
			if (LocalViewHeap) { delete(LocalViewHeap); }
		}

		inline ID3D12DescriptorHeap* GetViewDescriptorHeap()
		{
			return CurrentViewHeap->GetHeap();
		}

		inline ID3D12DescriptorHeap* GetSamplerDescriptorHeap()
		{
			return CurrentSamplerHeap->GetHeap();
		}

		// Checks if the specified descriptor heap has been set on the current command list.
		bool IsHeapSet(ID3D12DescriptorHeap* const pHeap) const
		{
			return (pHeap == pPreviousViewHeap) || (pHeap == pPreviousSamplerHeap);
		}

		// Notify the descriptor cache every time you start recording a command list.
		// This sets descriptor heaps on the command list and indicates the current fence value which allows
		// us to avoid querying DX12 for that value thousands of times per frame, which can be costly.
		void NotifyCurrentCommandList(ID3D12GraphicsCommandList& CommandListHandle);

		// ------------------------------------------------------
		// end Descriptor Slot Reservation stuff

		// null views

		D3D12_CPU_DESCRIPTOR_HANDLE* pNullSRV;
		D3D12_CPU_DESCRIPTOR_HANDLE* pNullRTV;
		D3D12_CPU_DESCRIPTOR_HANDLE* pNullUAV;

		D3D12_CPU_DESCRIPTOR_HANDLE* pDefaultSampler;

		void SetVertexBuffers(VertexBufferCache& Cache);
		void SetRenderTargets(RenderTargetView** RenderTargetViewArray, uint32 Count, DepthStencilView* DepthStencilTarget);

		template <ShaderType ShaderStage>
		void SetUAVs(const RootSignature* RootSignature, UnorderedAccessViewCache& Cache, const UAVSlotMask& SlotsNeededMask, uint32 Count, uint32& HeapSlot);

		template <ShaderType ShaderStage>
		void SetSamplers(const RootSignature* RootSignature, SamplerStateCache& Cache, const SamplerSlotMask& SlotsNeededMask, uint32 Count, uint32& HeapSlot);

		template <ShaderType ShaderStage>
		void SetSRVs(const RootSignature* RootSignature, ShaderResourceViewCache& Cache, const SRVSlotMask& SlotsNeededMask, uint32 Count, uint32& HeapSlot);

		template <ShaderType ShaderStage>
		void SetConstantBuffers(const RootSignature* RootSignature, ConstantBufferCache& Cache, const CBVSlotMask& SlotsNeededMask);

		void SetStreamOutTargets(ResourceHolder** Buffers, uint32 Count, const uint32* Offsets);

		bool HeapRolledOver(D3D12_DESCRIPTOR_HEAP_TYPE Type);
		void HeapLoopedAround(D3D12_DESCRIPTOR_HEAP_TYPE Type);
		void Init(Device* InParent, CommandContext* InCmdContext, uint32 InNumLocalViewDescriptors, uint32 InNumSamplerDescriptors, SubAllocatedOnlineHeap::SubAllocationDesc& SubHeapDesc);
		void Clear();
		void BeginFrame();
		void EndFrame();
		void GatherUniqueSamplerTables();

		bool SwitchToContextLocalViewHeap(ID3D12GraphicsCommandList& CommandListHandle);
		bool SwitchToContextLocalSamplerHeap();
		bool SwitchToGlobalSamplerHeap();

		std::vector<UniqueSamplerTable>& GetUniqueTables() { return UniqueTables; }

		inline bool UsingGlobalSamplerHeap() const { return bUsingGlobalSamplerHeap; }
		SamplerSet& GetLocalSamplerSet() { return LocalSamplerSet; }

	private:
		// Sets the current descriptor tables on the command list and marks any descriptor tables as dirty if necessary.
		// Returns true if one of the heaps actually changed, false otherwise.
		bool SetDescriptorHeaps();

		// The previous view and sampler heaps set on the current command list.
		ID3D12DescriptorHeap* pPreviousViewHeap;
		ID3D12DescriptorHeap* pPreviousSamplerHeap;

		OnlineHeap* CurrentViewHeap;
		OnlineHeap* CurrentSamplerHeap;

		ThreadLocalOnlineHeap* LocalViewHeap;
		ThreadLocalOnlineHeap LocalSamplerHeap;
		SubAllocatedOnlineHeap SubAllocatedViewHeap;

		SamplerMap SamplerMap;

		std::vector<UniqueSamplerTable> UniqueTables;

		SamplerSet LocalSamplerSet;
		bool bUsingGlobalSamplerHeap;

		uint32 NumLocalViewDescriptors;
	};
}
