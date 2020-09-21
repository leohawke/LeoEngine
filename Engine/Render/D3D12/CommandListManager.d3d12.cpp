#include "NodeDevice.h"
#include "CommandListManager.h"
#include "Fence.h"
#include "Adapter.h"

using namespace platform_ex::Windows::D3D12;

CommandAllocatorManager::CommandAllocatorManager(NodeDevice* InParent, const D3D12_COMMAND_LIST_TYPE& InType)
	:DeviceChild(InParent), Type(InType)
{
}

CommandAllocatorManager::~CommandAllocatorManager()
{
	for (auto pCommandAllocator : CommandAllocators)
	{
		delete pCommandAllocator;
	}
}

CommandAllocator* CommandAllocatorManager::ObtainCommandAllocator()
{
	std::unique_lock Lock{ CS };

	// See if the first command allocator in the queue is ready to be reset (will check associated fence)
	CommandAllocator* pCommandAllocator = CommandAllocatorQueue.empty() ? nullptr:CommandAllocatorQueue.front();

	if (pCommandAllocator && pCommandAllocator->IsReady())
	{
		pCommandAllocator->Reset();
		CommandAllocatorQueue.pop();
	}
	else
	{
		// The queue was empty, or no command allocators were ready, so create a new command allocator.
		pCommandAllocator = new CommandAllocator(GetParentDevice()->GetDevice(), Type);
		CommandAllocators.emplace_back(pCommandAllocator);

		// Set a valid sync point
		Fence& FrameFence = GetParentDevice()->GetParentAdapter()->GetFrameFence();
		const SyncPoint SyncPoint(&FrameFence, FrameFence.UpdateLastCompletedFence());
		pCommandAllocator->SetSyncPoint(SyncPoint);
	}

	return pCommandAllocator;
}

void CommandAllocatorManager::ReleaseCommandAllocator(CommandAllocator* pCommandAllocator)
{
	std::unique_lock Lock{ CS };

	CommandAllocatorQueue.push(pCommandAllocator);
}

void CommandListPayload::Reset()
{
	NumCommandLists = 0;
	std::fill_n(CommandLists, MaxCommandListsPerPayload, nullptr);
}

void CommandListPayload::Append(ID3D12CommandList* CommandList)
{
	CommandLists[NumCommandLists] = CommandList;
	++NumCommandLists;
}

CommandListManager::CommandListManager(NodeDevice* InParent, D3D12_COMMAND_LIST_TYPE InCommandListType, CommandQueueType InQueueType)
	:DeviceChild(InParent)
	,SingleNodeGPUObject(InParent->GetGPUMask())
	,ResourceBarrierCommandAllocator(nullptr)
	,CommandListFence(nullptr)
	,CommandListType(InCommandListType)
	,QueueType(InQueueType)
{
}

CommandListManager::~CommandListManager()
{
	Destroy();
}

void CommandListManager::Destroy()
{
	// Wait for the queue to empty
	WaitForCommandQueueFlush();

	{
		CommandListHandle hList;
		while (!ReadyLists.empty())
		{
			hList = ReadyLists.front();
			ReadyLists.pop();
		}
	}

	D3DCommandQueue = nullptr;

	if (CommandListFence)
	{
		CommandListFence->Destroy();
		CommandListFence.reset();
	}
}

void CommandListManager::Create(const std::string_view& Name, uint32 NumCommandLists, uint32 Priority)
{
	auto Device = GetParentDevice();
	auto Adapter = Device->GetParentAdapter();

	CommandListFence.reset(new Fence(Adapter, GetGPUMask(), "Command List Fence"));
	CommandListFence->CreateFence();

	bool bFullGPUCrashDebugging = false;

	D3D12_COMMAND_QUEUE_DESC CommandQueueDesc = {};
	CommandQueueDesc.Flags = (bFullGPUCrashDebugging)
		? D3D12_COMMAND_QUEUE_FLAG_DISABLE_GPU_TIMEOUT : D3D12_COMMAND_QUEUE_FLAG_NONE;
	CommandQueueDesc.NodeMask = GetGPUMask();
	CommandQueueDesc.Priority = Priority;
	CommandQueueDesc.Type = CommandListType;
	CheckHResult(Adapter->GetDevice()->CreateCommandQueue(&CommandQueueDesc,IID_PPV_ARGS(D3DCommandQueue.ReleaseAndGetAddress())));

	D3D::Debug(D3DCommandQueue, Name.data());

	if (NumCommandLists > 0)
	{
		// Create a temp command allocator for command list creation.
		CommandAllocator TempCommandAllocator(Device->GetDevice(), CommandListType);
		for (uint32 i = 0; i < NumCommandLists; ++i)
		{
			CommandListHandle hList = CreateCommandListHandle(TempCommandAllocator);
			ReadyLists.push(hList);
		}
	}
}

CommandListHandle CommandListManager::ObtainCommandList(CommandAllocator& CommandAllocator)
{
	std::unique_lock Lock(ReadyListsCS);

	CommandListHandle List;
	if (ReadyLists.empty())
	{
		List = CreateCommandListHandle(CommandAllocator);
	}
	else 
	{
		List = ReadyLists.front();
		ReadyLists.pop();
	}

	lconstraint(List.GetCommandListType() == CommandListType);

	List.Reset(CommandAllocator,false);

	return List;
}

void CommandListManager::ReleaseCommandList(CommandListHandle& hList)
{
	lconstraint(hList.IsClosed());
	lconstraint(hList.GetCommandListType() == CommandListType);

	hList.CurrentCommandAllocator()->DecrementPendingCommandLists();

	std::unique_lock Lock(ReadyListsCS);
	ReadyLists.push(hList);
}

CommandListHandle CommandListManager::CreateCommandListHandle(CommandAllocator& CommandAllocator)
{
	CommandListHandle List;
	List.Create(GetParentDevice(), CommandListType, CommandAllocator, this);
	return List;
}

void CommandListManager::ExecuteCommandList(CommandListHandle& hList, bool WaitForCompletion)
{
	std::vector<CommandListHandle> Lists;
	Lists.emplace_back(hList);

	ExecuteCommandLists(Lists, WaitForCompletion);
}

uint64 CommandListManager::ExecuteAndIncrementFence(CommandListPayload& Payload, Fence& Fence)
{
	std::unique_lock Lock{ FenceCS };

	D3DCommandQueue->ExecuteCommandLists(Payload.NumCommandLists, Payload.CommandLists);

	LAssert(Fence.GetGPUMask() == GetGPUMask(), "Fence GPU masks does not fit with the command list mask!");

	return Fence.Signal(QueueType);
}

void CommandListManager::ExecuteCommandLists(std::vector<CommandListHandle>& Lists, bool WaitForCompletion)
{
	bool NeedsResourceBarriers = false;
	for (int32 i = 0; i < Lists.size(); i++)
	{
		auto& commandList = Lists[i];
		//if (commandList.PendingResourceBarriers().Num() > 0)
		{
			//NeedsResourceBarriers = true;
			break;
		}
	}

	uint64 SignaledFenceValue = -1;
	uint64 BarrierFenceValue = -1;
	D3D12::SyncPoint SyncPoint;
	D3D12::SyncPoint BarrierSyncPoint;

	auto& DirectCommandListManager = GetParentDevice()->GetCommandListManager();
	auto& DirectFence = DirectCommandListManager.GetFence();

	LAssert(DirectFence.GetGPUMask() == GetGPUMask(), "Fence GPU masks does not fit with the command list mask!");

	int32 commandListIndex = 0;
	int32 barrierCommandListIndex = 0;

	// Close the resource barrier lists, get the raw command list pointers, and enqueue the command list handles
	// Note: All command lists will share the same fence
	CommandListPayload CurrentCommandListPayload;
	CommandListPayload ComputeBarrierPayload;

	lconstraint(Lists.size() <= CommandListPayload::MaxCommandListsPerPayload);

	CommandListHandle BarrierCommandList[128];
	if (NeedsResourceBarriers)
	{

	}
	else
	{
		for (int32 i = 0; i < Lists.size(); i++)
		{
			CurrentCommandListPayload.Append(Lists[i].CommandList());
		}
		SignaledFenceValue = ExecuteAndIncrementFence(CurrentCommandListPayload, *CommandListFence);
		//check(CommandListType != D3D12_COMMAND_LIST_TYPE_COMPUTE);
		SyncPoint = D3D12::SyncPoint(CommandListFence.get(), SignaledFenceValue);
		BarrierSyncPoint = SyncPoint;
	}

	for (int32 i = 0; i < Lists.size(); i++)
	{
		auto& commandList = Lists[i];

		// Set a sync point on the command list so we know when it's current generation is complete on the GPU, then release it so it can be reused later.
		// Note this also updates the command list's command allocator
		commandList.SetSyncPoint(SyncPoint);
		ReleaseCommandList(commandList);
	}

	for (int32 i = 0; i < barrierCommandListIndex; i++)
	{
		auto& commandList = BarrierCommandList[i];

		// Set a sync point on the command list so we know when it's current generation is complete on the GPU, then release it so it can be reused later.
		// Note this also updates the command list's command allocator
		commandList.SetSyncPoint(BarrierSyncPoint);
		DirectCommandListManager.ReleaseCommandList(commandList);
	}

	if (WaitForCompletion)
	{
		CommandListFence->WaitForFence(SignaledFenceValue);
		lconstraint(SyncPoint.IsComplete());
	}
}

void CommandListManager::WaitForCommandQueueFlush()
{
	if (D3DCommandQueue)
	{
		auto SignalFence = CommandListFence->Signal(QueueType);

		CommandListFence->WaitForFence(SignalFence);
	}
}