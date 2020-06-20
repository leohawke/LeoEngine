#pragma once

#include "Common.h"
#include "D3DCommandList.h"

namespace platform_ex::Windows::D3D12
{
	class CommandListManager : public DeviceChild, public SingleNodeGPUObject
	{
	public:
		CommandListManager(NodeDevice* InParent, D3D12_COMMAND_LIST_TYPE InCommandListType, CommandQueueType InQueueType);
		virtual ~CommandListManager();

		// This use to also take an optional PSO parameter so that we could pass this directly to Create/Reset command lists,
		// however this was removed as we generally can't actually predict what PSO we'll need until draw due to frequent
		// state changes. We leave PSOs to always be resolved in ApplyState().
		CommandListHandle ObtainCommandList(CommandAllocator& CommandAllocator);
		void ReleaseCommandList(CommandListHandle& hList);
	};
}