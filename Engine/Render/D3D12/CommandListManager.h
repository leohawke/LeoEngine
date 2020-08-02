#pragma once

#include "Common.h"
#include "D3DCommandList.h"
#include <queue>
#include <mutex>

namespace platform_ex::Windows::D3D12
{
	struct CommandListPayload
	{
		CommandListPayload()
		{
			NumCommandLists = 0;
			std::fill_n(CommandLists, MaxCommandListsPerPayload, nullptr);
		}

		void Reset();
		void Append(ID3D12CommandList* CL);

		static constexpr uint32 MaxCommandListsPerPayload = 256;
		ID3D12CommandList* CommandLists[MaxCommandListsPerPayload];
		uint32 NumCommandLists;
	};

	class CommandListManager : public DeviceChild, public SingleNodeGPUObject
	{
	public:
		CommandListManager(NodeDevice* InParent, D3D12_COMMAND_LIST_TYPE InCommandListType, CommandQueueType InQueueType);
		virtual ~CommandListManager();

		void Create(const std::string_view& Name, uint32 NumCommandLists = 0, uint32 Priority = 0);
		void Destroy();

		// This use to also take an optional PSO parameter so that we could pass this directly to Create/Reset command lists,
		// however this was removed as we generally can't actually predict what PSO we'll need until draw due to frequent
		// state changes. We leave PSOs to always be resolved in ApplyState().
		CommandListHandle ObtainCommandList(CommandAllocator& CommandAllocator);
		void ReleaseCommandList(CommandListHandle& hList);

		ID3D12CommandQueue* GetD3DCommandQueue() const
		{
			return D3DCommandQueue.Get();
		}

		Fence& GetFence() { return *CommandListFence; }

		void ExecuteCommandList(CommandListHandle& hList, bool WaitForCompletion);

		uint64 ExecuteAndIncrementFence(CommandListPayload& Payload, Fence& Fence);

		void ExecuteCommandLists(std::vector<CommandListHandle>& Lists, bool WaitForCompletion);
	protected:
		void WaitForCommandQueueFlush();

		CommandListHandle CreateCommandListHandle(CommandAllocator& CommandAllocator);

		COMPtr<ID3D12CommandQueue> D3DCommandQueue;
		std::queue<CommandListHandle> ReadyLists;
		std::mutex					  ReadyListsCS;

		CommandAllocator* ResourceBarrierCommandAllocator;

		std::shared_ptr<Fence> CommandListFence;

		D3D12_COMMAND_LIST_TYPE					CommandListType;
		CommandQueueType						QueueType;

		std::mutex								FenceCS;
	};
}