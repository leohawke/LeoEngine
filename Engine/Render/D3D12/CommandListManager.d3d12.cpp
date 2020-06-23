#include "NodeDevice.h"
#include "CommandListManager.h"
#include "Fence.h"
#include "Adapter.h"

using namespace platform_ex::Windows::D3D12;

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

void CommandListManager::WaitForCommandQueueFlush()
{
	if (D3DCommandQueue)
	{
		auto SignalFence = CommandListFence->Signal(QueueType);

		CommandListFence->WaitForFence(SignalFence);
	}
}