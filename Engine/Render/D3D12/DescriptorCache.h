#pragma once

#include <LBase/ldef.h>
#include "Common.h"
#include "d3d12_dxgi.h"
#include <unordered_set>
#include "../ShaderCore.h"

namespace platform_ex::Windows::D3D12 {
	class CommandContext;

	using platform::Render::ShaderType;

	// Like a std::unordered_map<KeyType, ValueType>
	// Faster lookup performance, but possibly has false negatives
	template<typename KeyType, typename ValueType>
	class ConservativeMap
	{

	};

	struct SamplerArrayDesc
	{

	};

	using SamplerMap = ConservativeMap<SamplerArrayDesc, D3D12_GPU_DESCRIPTOR_HANDLE>;

	template< uint32 CPUTableSize>
	struct UniqueDescriptorTable
	{

	};

	using UniqueSamplerTable = UniqueDescriptorTable<MAX_SAMPLERS>;

	using SamplerSet = std::unordered_set<UniqueSamplerTable>;

	class OnlineHeap
	{
	public:
		ID3D12DescriptorHeap* GetHeap()
		{
			return nullptr;
		}
	};

	class SubAllocatedOnlineHeap
	{

	};

	class ThreadLocalOnlineHeap
	{

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
		void NotifyCurrentCommandList(const ID3D12GraphicsCommandList& CommandListHandle);

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

		bool SwitchToContextLocalViewHeap(const ID3D12GraphicsCommandList& CommandListHandle);
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
