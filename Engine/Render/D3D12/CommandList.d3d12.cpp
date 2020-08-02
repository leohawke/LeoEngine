#include "NodeDevice.h"
#include "D3DCommandList.h"
#include "CommandListManager.h"
#include "CommandContext.h"

using namespace platform_ex::Windows::D3D12;

CommandListHandle::CommandListData::CommandListData(NodeDevice* ParentDevice, D3D12_COMMAND_LIST_TYPE InCommandListType, CommandAllocator& CommandAllocator, D3D12::CommandListManager* InCommandListManager)
	:DeviceChild(ParentDevice)
	,SingleNodeGPUObject(ParentDevice->GetGPUMask())
	,NumRefs(1)
	,CommandListManager(InCommandListManager)
	,CurrentOwningContext(nullptr)
	,CommandListType(InCommandListType)
	,CurrentGeneration(1)
	,LastCompleteGeneration(0)
	,IsClosed(false)
{
	CheckHResult(ParentDevice->GetDevice()->CreateCommandList(GetGPUMask(), CommandListType, CommandAllocator, nullptr, IID_PPV_ARGS(CommandList.ReleaseAndGetAddress())));

	// Optionally obtain the ID3D12GraphicsCommandList1 & ID3D12GraphicsCommandList2 interface, we don't check the HRESULT.
	CommandList->QueryInterface(IID_PPV_ARGS(CommandList1.ReleaseAndGetAddress()));
	CommandList->QueryInterface(IID_PPV_ARGS(CommandList2.ReleaseAndGetAddress()));

	CommandList->QueryInterface(IID_PPV_ARGS(RayTracingCommandList.ReleaseAndGetAddress()));

	auto name = leo::sfmt("CommandListHandle (GPU %u)",ParentDevice->GetGPUIndex());

	D3D::Debug(CommandList, name.c_str());

	Close();
}

CommandListHandle::CommandListData::~CommandListData()
{
	CommandList = nullptr;
}

void CommandListHandle::CommandListData::Close()
{
	if (!IsClosed)
	{
		FlushResourceBarriers();

		CheckHResult(CommandList->Close());

		IsClosed = true;
	}
}

void CommandListHandle::CommandListData::FlushResourceBarriers()
{
	ResourceBarrierBatcher.Flush(CommandList.Get());
}

void CommandListHandle::CommandListData::Reset(CommandAllocator& Allocator, bool bTrackExecTime)
{
	CheckHResult(CommandList->Reset(Allocator, nullptr));

	CurrentCommandAllocator = &Allocator;
	IsClosed = false;

	// Indicate this command allocator is being used.
	CurrentCommandAllocator->IncrementPendingCommandLists();

	CleanupActiveGenerations();
}

bool CommandListHandle::CommandListData::IsComplete(uint64 Generation)
{
	if (Generation >= CurrentGeneration)
	{
		// Have not submitted this generation for execution yet.
		return false;
	}

	lconstraint(Generation < CurrentGeneration);
	if (Generation > LastCompleteGeneration)
	{
		std::unique_lock Lock(ActiveGenerationsCS);
		if (!ActiveGenerations.empty())
		{
			GenerationSyncPointPair GenerationSyncPoint = ActiveGenerations.front();
			ActiveGenerations.pop();

			if (Generation < GenerationSyncPoint.first)
			{
				// The requested generation is older than the oldest tracked generation, so it must be complete.
				return true;
			}
			else
			{
				if (GenerationSyncPoint.second.IsComplete())
				{
					// Oldest tracked generation is done so clean the queue and try again.
					CleanupActiveGenerations();
					return IsComplete(Generation);
				}
				else
				{
					// The requested generation is newer than the older track generation but the old one isn't done.
					return false;
				}
			}
		}
	}

	return true;
}

void CommandListHandle::CommandListData::WaitForCompletion(uint64 Generation)
{
	if (Generation > LastCompleteGeneration)
	{
		CleanupActiveGenerations();
		if (Generation > LastCompleteGeneration)
		{
			std::unique_lock Lock(ActiveGenerationsCS);
			LAssert(Generation < CurrentGeneration, "You can't wait for an unsubmitted command list to complete.  Kick first!");
			GenerationSyncPointPair GenerationSyncPoint;
			while (!ActiveGenerations.empty()  && (Generation > LastCompleteGeneration))
			{
				lconstraint(Generation >= GenerationSyncPoint.first);
				GenerationSyncPoint = ActiveGenerations.front();
				ActiveGenerations.pop();

				// Unblock other threads while we wait for the command list to complete
				ActiveGenerationsCS.unlock();

				GenerationSyncPoint.second.WaitForCompletion();

				ActiveGenerationsCS.lock();
				LastCompleteGeneration = std::max(LastCompleteGeneration, GenerationSyncPoint.first);
			}
		}
	}
}

void CommandListHandle::CommandListData::SetSyncPoint(const SyncPoint& InSyncPoint)
{
	{
		std::unique_lock Lock(ActiveGenerationsCS);

		// Only valid sync points should be set otherwise we might not wait on the GPU correctly.
		lconstraint(InSyncPoint.IsValid());

		// Track when this command list generation is completed on the GPU.
		GenerationSyncPointPair CurrentGenerationSyncPoint;
		CurrentGenerationSyncPoint.first = CurrentGeneration;
		CurrentGenerationSyncPoint.second = InSyncPoint;
		ActiveGenerations.push(CurrentGenerationSyncPoint);

		// Move to the next generation of the command list.
		CurrentGeneration++;
	}

	// Update the associated command allocator's sync point so it's not reset until the GPU is done with all command lists using it.
	CurrentCommandAllocator->SetSyncPoint(InSyncPoint);
}

void CommandListHandle::CommandListData::CleanupActiveGenerations()
{
	std::unique_lock Lock(ActiveGenerationsCS);

	// Cleanup the queue of active command list generations.
	// Only remove them from the queue when the GPU has completed them.
	while (!ActiveGenerations.empty() && ActiveGenerations.front().second.IsComplete())
	{
		// The GPU is done with the work associated with this generation, remove it from the queue.
		auto GenerationSyncPoint = ActiveGenerations.front();
		ActiveGenerations.pop();

		lconstraint(GenerationSyncPoint.first > LastCompleteGeneration);
		LastCompleteGeneration = GenerationSyncPoint.first;
	}
}

void CommandListHandle::AddTransitionBarrier(ResourceHolder* pResource, D3D12_RESOURCE_STATES Before, D3D12_RESOURCE_STATES After, uint32 Subresource)
{
	if (Before != After)
	{
		int32 NumAdded = CommandListData->ResourceBarrierBatcher.AddTransition(pResource->Resource(), Before, After, Subresource);
		CommandListData->CurrentOwningContext->numBarriers += NumAdded;
	}
}

void platform_ex::Windows::D3D12::CommandListHandle::AddUAVBarrier()
{
	CommandListData->ResourceBarrierBatcher.AddUAV();
	++CommandListData->CurrentOwningContext->numBarriers;
}

void CommandListHandle::Create(NodeDevice* InParent, D3D12_COMMAND_LIST_TYPE InCommandType, CommandAllocator& InAllocator, CommandListManager* InManager)
{
	CommandListData = new D3D12CommandListData(InParent, InCommandType, InAllocator, InManager);
}

void CommandListHandle::Execute(bool WaitForCompletion)
{
	CommandListData->CommandListManager->ExecuteCommandList(*this, WaitForCompletion);
}

CommandAllocator::CommandAllocator(ID3D12Device* InDevice, const D3D12_COMMAND_LIST_TYPE& InType)
	:PendingCommandListCount(0)
{
	Init(InDevice, InType);
}

CommandAllocator::~CommandAllocator()
{
	D3DCommandAllocator = nullptr;
}

void CommandAllocator::Init(ID3D12Device* InDevice, const D3D12_COMMAND_LIST_TYPE& InType)
{
	CheckHResult(InDevice->CreateCommandAllocator(InType, IID_PPV_ARGS(D3DCommandAllocator.ReleaseAndGetAddress())));
}

void platform_ex::Windows::D3D12::TransitionResource(CommandListHandle& hCommandList, DepthStencilView* pView, D3D12_RESOURCE_STATES after)
{
	auto pResource = pView->GetResourceLocation();
	auto ResourceDesc = pResource->GetDesc();

	const D3D12_DEPTH_STENCIL_VIEW_DESC& desc = pView->GetDesc();
	switch (desc.ViewDimension)
	{
	case D3D12_DSV_DIMENSION_TEXTURE2D:
	case D3D12_DSV_DIMENSION_TEXTURE2DMS:
		if (GetPlaneCount(ResourceDesc.Format) > 1)
		{
			// Multiple subresources to transtion
			TransitionResource(hCommandList, pResource, after, pView->GetViewSubresourceSubset());
			break;
		}
		else
		{
			// Only one subresource to transition
			lconstraint(GetPlaneCount(ResourceDesc.Format) == 1);
			TransitionResource(hCommandList, pResource, after, desc.Texture2D.MipSlice);
		}
		break;

	case D3D12_DSV_DIMENSION_TEXTURE2DARRAY:
	case D3D12_DSV_DIMENSION_TEXTURE2DMSARRAY:
	{
		// Multiple subresources to transtion
		TransitionResource(hCommandList, pResource, after, pView->GetViewSubresourceSubset());
		break;
	}

	default:
		lconstraint(false);	// Need to update this code to include the view type
		break;
	}
}

void platform_ex::Windows::D3D12::TransitionResource(CommandListHandle& hCommandList, RenderTargetView* pView, D3D12_RESOURCE_STATES after)
{
	auto pResource = pView->GetResourceLocation();

	const D3D12_RENDER_TARGET_VIEW_DESC& desc = pView->GetDesc();
	switch (desc.ViewDimension)
	{
	case D3D12_RTV_DIMENSION_TEXTURE3D:
		// Note: For volume (3D) textures, all slices for a given mipmap level are a single subresource index.
		// Fall-through
	case D3D12_RTV_DIMENSION_TEXTURE2D:
	case D3D12_RTV_DIMENSION_TEXTURE2DMS:
		// Only one subresource to transition
		TransitionResource(hCommandList, pResource, after, desc.Texture2D.MipSlice);
		break;

	case D3D12_RTV_DIMENSION_TEXTURE2DARRAY:
	{
		// Multiple subresources to transition
		TransitionResource(hCommandList, pResource, after, pView->GetViewSubresourceSubset());
		break;
	}

	default:
		lconstraint(false);	// Need to update this code to include the view type
		break;
	}
}