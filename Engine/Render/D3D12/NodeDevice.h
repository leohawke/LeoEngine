#pragma once

#include <LBase/id.hpp>
#include "Common.h"
#include "d3d12_dxgi.h"
#include "View.h"
#include "DescriptorCache.h"

#include <vector>
#include <unordered_map>


struct SamplerDescHash
{
	static_assert(sizeof(D3D12_SAMPLER_DESC) % 4 == 0);

	std::size_t operator()(const D3D12_SAMPLER_DESC& desc) const noexcept {
		auto ptr = reinterpret_cast<const leo::uint32*>(&desc);
		return leo::hash(ptr, ptr + sizeof(desc));
	}
};

inline bool operator==(const D3D12_SAMPLER_DESC& lhs, const D3D12_SAMPLER_DESC& rhs)
{
	return std::memcmp(&lhs, &rhs, sizeof(lhs)) == 0;
}

namespace platform_ex::Windows::D3D12
{
	class CommandContext;

	class NodeDevice : public SingleNodeGPUObject, public AdapterChild
	{
	public:
		NodeDevice(GPUMaskType InGpuMask, D3D12Adapter* InAdapter);

		void Initialize();

		void CreateCommandContexts();

		ID3D12Device* GetDevice();

		template <typename TViewDesc> OfflineDescriptorManager& GetViewDescriptorAllocator();
		template<> OfflineDescriptorManager& GetViewDescriptorAllocator<D3D12_SHADER_RESOURCE_VIEW_DESC>() { return SRVAllocator; }
		template<> OfflineDescriptorManager& GetViewDescriptorAllocator<D3D12_RENDER_TARGET_VIEW_DESC>() { return RTVAllocator; }
		template<> OfflineDescriptorManager& GetViewDescriptorAllocator<D3D12_DEPTH_STENCIL_VIEW_DESC>() { return DSVAllocator; }
		template<> OfflineDescriptorManager& GetViewDescriptorAllocator<D3D12_UNORDERED_ACCESS_VIEW_DESC>() { return UAVAllocator; }

		OfflineDescriptorManager& GetSamplerDescriptorAllocator() { return SamplerAllocator; }

		CommandContext& GetDefaultCommandContext()
		{
			return *CommandContextArray[0];
		}

		ID3D12CommandQueue* GetD3DCommandQueue(CommandQueueType InQueueType);

		CommandListManager* GetCommandListManager(CommandQueueType InQueueType);

		CommandListManager& GetCommandListManager() const { return *CommandListManager; }

		void CreateSamplerInternal(const D3D12_SAMPLER_DESC& Desc, D3D12_CPU_DESCRIPTOR_HANDLE Descriptor);

		std::shared_ptr<SamplerState> CreateSampler(const D3D12_SAMPLER_DESC& Desc);

		inline GlobalOnlineHeap& GetGlobalSamplerHeap() { return GlobalSamplerHeap; }
	private:
		void SetupAfterDeviceCreation();
	protected:
		using D3D12CommandListManager = CommandListManager;
		/** A pool of command lists we can cycle through for the global D3D device */
		D3D12CommandListManager* CommandListManager;
		D3D12CommandListManager* CopyCommandListManager;
		D3D12CommandListManager* AsyncCommandListManager;

		// Must be before the StateCache so that destructor ordering is valid
		OfflineDescriptorManager RTVAllocator;
		OfflineDescriptorManager DSVAllocator;
		OfflineDescriptorManager SRVAllocator;
		OfflineDescriptorManager UAVAllocator;
		OfflineDescriptorManager SamplerAllocator;

		std::vector<CommandContext*> CommandContextArray;

		std::unordered_map<D3D12_SAMPLER_DESC, std::shared_ptr<SamplerState>, SamplerDescHash> SamplerMap;
		uint32 SamplerID;

		GlobalOnlineHeap GlobalSamplerHeap;
		GlobalOnlineHeap GlobalViewHeap;
	};


	template<typename TDesc>
	void TViewDescriptorHandle<TDesc>::AllocateDescriptorSlot()
	{
		if (Parent)
		{
			auto Device = GetParentDevice();
			auto& DescriptorAllocator = Device->template GetViewDescriptorAllocator<TDesc>();
			Handle = DescriptorAllocator.AllocateHeapSlot(Index);
			lconstraint(Handle.ptr != 0);
		}
	}

	template <typename TDesc>
	void TViewDescriptorHandle<TDesc>::FreeDescriptorSlot()
	{
		if (Parent)
		{
			auto Device = GetParentDevice();
			auto& DescriptorAllocator = Device->template GetViewDescriptorAllocator<TDesc>();
			DescriptorAllocator.FreeHeapSlot(Handle, Index);
			Handle.ptr = 0;
		}
		lconstraint(!Handle.ptr);
	}

	template<typename TDesc>
	void TViewDescriptorHandle<TDesc>::CreateView(const TDesc& Desc, ID3D12Resource* Resource)
	{
		// NOTE (from D3D Debug runtime): When ViewDimension is D3D12_SRV_DIMENSION_RAYTRACING_ACCELLERATION_STRUCTURE, pResource must be NULL, since the resource location comes from a GPUVA in pDesc
		if constexpr (std::is_same_v<TDesc, D3D12_SHADER_RESOURCE_VIEW_DESC>)
		{
			if (Desc.ViewDimension == D3D12_SRV_DIMENSION_RAYTRACING_ACCELERATION_STRUCTURE)
			{
				Resource = nullptr;
			}
		}

		(GetParentDevice()->GetDevice()->*TCreateViewMap<TDesc>::GetCreate()) (Resource, &Desc, Handle);
	}

	template<typename TDesc>
	void TViewDescriptorHandle<TDesc>::CreateViewWithCounter(const TDesc& Desc, ID3D12Resource* Resource, ID3D12Resource* CounterResource)
	{
		(GetParentDevice()->GetDevice()->*TCreateViewMap<TDesc>::GetCreate()) (Resource, CounterResource, &Desc, Handle);
	}
}