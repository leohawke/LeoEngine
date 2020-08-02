#pragma once

#include "Utility.h"
#include "Common.h"
#include "ResourceHolder.h"
#include <atomic>
#include <queue>
#include <mutex>

namespace platform_ex::Windows::D3D12 {
	class UnorderedAccessView;
	class ShaderResourceView;
	class RenderTargetView;
	class DepthStencilView;
	class ResourceHolder;

	class CommandContext;
	class CommandListManager;

	class CommandAllocator
	{
	public:
		explicit CommandAllocator(ID3D12Device* InDevice, const D3D12_COMMAND_LIST_TYPE& InType);
		~CommandAllocator();

		// The command allocator is ready to be reset when all command lists have been executed (or discarded) AND the GPU not using it.
		inline bool IsReady() const { return (PendingCommandListCount == 0) && SyncPoint.IsComplete(); }
		inline bool HasValidSyncPoint() const { return SyncPoint.IsValid(); }
		inline void SetSyncPoint(const SyncPoint& InSyncPoint) { lconstraint(InSyncPoint.IsValid()); SyncPoint = InSyncPoint; }
		inline void Reset() { lconstraint(IsReady()); CheckHResult(D3DCommandAllocator->Reset()); }

		operator ID3D12CommandAllocator* () { return D3DCommandAllocator.Get(); }

		// Called to indicate a command list is using this command alloctor
		inline void IncrementPendingCommandLists()
		{
			lconstraint(PendingCommandListCount >= 0);
			++PendingCommandListCount;
		}

		// Called to indicate a command list using this allocator has been executed OR discarded (closed with no intention to execute it).
		inline void DecrementPendingCommandLists()
		{
			lconstraint(PendingCommandListCount > 0);
			--PendingCommandListCount;
		}

	private:
		void Init(ID3D12Device* InDevice, const D3D12_COMMAND_LIST_TYPE& InType);

	private:
		COMPtr<ID3D12CommandAllocator> D3DCommandAllocator;
		SyncPoint SyncPoint;	// Indicates when the GPU is finished using the command allocator.
		std::atomic<int32> PendingCommandListCount;	// The number of command lists using this allocator but haven't been executed yet.
	};

	class CommandListHandle
	{
	private:
		class CommandListData :public DeviceChild, public SingleNodeGPUObject
		{
		public:
			CommandListData(NodeDevice* ParentDevice, D3D12_COMMAND_LIST_TYPE InCommandListType, CommandAllocator& CommandAllocator, CommandListManager* InCommandListManager);

			~CommandListData();

			void Close();

			bool IsComplete(uint64 Generation);

			void WaitForCompletion(uint64 Generation);

			void SetSyncPoint(const SyncPoint& InSyncPoint);

			void FlushResourceBarriers();

			void Reset(CommandAllocator& Allocator, bool bTrackExecTime = false);


			uint32 AddRef() const
			{
				return ++NumRefs;
			}

			uint32 Release() const
			{
				return --NumRefs;
			}

			mutable std::atomic<uint32>	NumRefs;

			CommandListManager* CommandListManager;
			CommandContext* CurrentOwningContext;
			const D3D12_COMMAND_LIST_TYPE			CommandListType;
			COMPtr<ID3D12GraphicsCommandList>	CommandList;		// Raw D3D command list pointer
			COMPtr<ID3D12GraphicsCommandList1>	CommandList1;
			COMPtr<ID3D12GraphicsCommandList2>	CommandList2;
			COMPtr<ID3D12GraphicsCommandList4>	RayTracingCommandList;

			CommandAllocator* CurrentCommandAllocator;	// Command allocator currently being used for recording the command list

			uint64									CurrentGeneration;
			uint64									LastCompleteGeneration;

			bool									IsClosed;

			using GenerationSyncPointPair = std::pair<uint64, SyncPoint>;
			std::queue<GenerationSyncPointPair> ActiveGenerations;
			std::mutex ActiveGenerationsCS;

			ResourceBarrierBatcher ResourceBarrierBatcher;
		private:
			void CleanupActiveGenerations();
		};

		using D3D12CommandListData = CommandListData;

	public:
		CommandListHandle() : CommandListData(nullptr) {}

		CommandListHandle(const CommandListHandle& CL)
			: CommandListHandle(CL.CommandListData)
		{}

		CommandListHandle(CommandListData* InData)
			: CommandListData(InData)
		{
			if (CommandListData)
			{
				CommandListData->AddRef();
			}
		}

		CommandListHandle(CommandListHandle&& CL)
			: CommandListData(CL.CommandListData)
		{
			CL.CommandListData = nullptr;
		}

		virtual ~CommandListHandle()
		{
			if (CommandListData && CommandListData->Release() == 0)
			{
				delete CommandListData;
			}
		}

		CommandListHandle& operator = (const CommandListHandle& CL)
		{
			if (this != &CL)
			{
				if (CommandListData && CommandListData->Release() == 0)
				{
					delete CommandListData;
				}

				CommandListData = nullptr;

				if (CL.CommandListData)
				{
					CommandListData = CL.CommandListData;
					CommandListData->AddRef();
				}
			}

			return *this;
		}

		CommandListHandle& operator=(CommandListHandle&& CL)
		{
			if (CommandListData != CL.CommandListData)
			{
				if (CommandListData && CommandListData->Release() == 0)
				{
					delete CommandListData;
				}
				CommandListData = CL.CommandListData;
				CL.CommandListData = nullptr;
			}
			return *this;
		}

		uint64 CurrentGeneration() const
		{
			return CommandListData->CurrentGeneration;
		}

		void WaitForCompletion(uint64 Generation) const
		{
			return CommandListData->WaitForCompletion(Generation);
		}

		bool IsComplete(uint64 Generation) const
		{
			return CommandListData->IsComplete(Generation);
		}

		void SetSyncPoint(const SyncPoint& InSyncPoint)
		{
			return CommandListData->SetSyncPoint(InSyncPoint);
		}

		ID3D12CommandList* CommandList() const
		{
			return CommandListData->CommandList.Get();
		}

		ID3D12GraphicsCommandList* GraphicsCommandList() const
		{
			lconstraint(CommandListData->CommandListType == D3D12_COMMAND_LIST_TYPE_DIRECT || CommandListData->CommandListType == D3D12_COMMAND_LIST_TYPE_COMPUTE);

			return static_cast<ID3D12GraphicsCommandList*>(CommandList());
		}

		ID3D12GraphicsCommandList4* RayTracingCommandList() const
		{
			lconstraint(CommandListData && (CommandListData->CommandListType == D3D12_COMMAND_LIST_TYPE_DIRECT || CommandListData->CommandListType == D3D12_COMMAND_LIST_TYPE_COMPUTE));
			return CommandListData->RayTracingCommandList.Get();
		}

		void SetCurrentOwningContext(CommandContext* context)
		{
			CommandListData->CurrentOwningContext = context;
		}

		void Close()
		{
			CommandListData->Close();
		}

		bool IsClosed() const
		{
			return CommandListData->IsClosed;
		}

		D3D12_COMMAND_LIST_TYPE GetCommandListType() const
		{
			return CommandListData->CommandListType;
		}

		void Reset(CommandAllocator& Allocator, bool bTrackExecTime = false)
		{
			CommandListData->Reset(Allocator, bTrackExecTime);
		}

		CommandAllocator* CurrentCommandAllocator() const
		{
			return CommandListData->CurrentCommandAllocator;
		}

		void FlushResourceBarriers()
		{
			CommandListData->FlushResourceBarriers();
		}

		void AddTransitionBarrier(ResourceHolder* pResource, D3D12_RESOURCE_STATES Before, D3D12_RESOURCE_STATES After, uint32 Subresource);

		void AddUAVBarrier();

		void Create(NodeDevice* InParent, D3D12_COMMAND_LIST_TYPE InCommandType, CommandAllocator& InAllocator, CommandListManager* InManager);

		void Execute(bool WaitForCompletion = false);

		friend bool operator==(const CommandListHandle& lhs, std::nullptr_t)
		{
			return lhs.CommandListData == nullptr;
		}

		friend bool operator!=(const CommandListHandle& lhs, std::nullptr_t)
		{
			return !(lhs == nullptr);
		}

		ID3D12GraphicsCommandList* operator->() const
		{
			lconstraint(CommandListData && !CommandListData->IsClosed);

			return CommandListData->CommandList.Get();
		}
	private:
		CommandListHandle& operator*()
		{
			return *this;
		}

		CommandListData* CommandListData;
	};

	class CLSyncPoint
	{
	public:
		CLSyncPoint() : Generation(0) {}

		CLSyncPoint(CommandListHandle& CL) : CommandList(CL), Generation(CL.CommandList() ? CL.CurrentGeneration() : 0) {}

		CLSyncPoint(const CLSyncPoint& SyncPoint) : CommandList(SyncPoint.CommandList), Generation(SyncPoint.Generation) {}

		CLSyncPoint& operator = (CommandListHandle& CL)
		{
			CommandList = CL;
			Generation = (CL != nullptr) ? CL.CurrentGeneration() : 0;

			return *this;
		}

		CLSyncPoint& operator = (const CLSyncPoint& SyncPoint)
		{
			CommandList = SyncPoint.CommandList;
			Generation = SyncPoint.Generation;

			return *this;
		}

		bool operator!() const
		{
			return CommandList == 0;
		}

		bool IsValid() const
		{
			return CommandList != nullptr;
		}

		bool IsOpen() const
		{
			return Generation == CommandList.CurrentGeneration();
		}

		bool IsComplete() const
		{
			return CommandList.IsComplete(Generation);
		}

		void WaitForCompletion() const
		{
			CommandList.WaitForCompletion(Generation);
		}

		uint64 GetGeneration() const
		{
			return Generation;
		}
	private:
		friend class CommandListManager;

		CommandListHandle CommandList;
		uint64                  Generation;
	};

	//TODO SubresourceSubset
	inline void TransitionResource(CommandListHandle& pCommandList, UnorderedAccessView* View, D3D12_RESOURCE_STATES after)
	{
		//TODO
	}

	inline void TransitionResource(CommandListHandle& pCommandList, ShaderResourceView* View, D3D12_RESOURCE_STATES after)
	{
		//TODO
	}

	inline void TransitionResource(CommandListHandle& hCommandList, ResourceHolder* Resource, D3D12_RESOURCE_STATES after, const CViewSubresourceSubset& subresourceSubset)
	{
		const bool bIsWholeResource = subresourceSubset.IsWholeResource();

		if (bIsWholeResource) {
			if (Resource->IsTransitionNeeded(after))
			{
				hCommandList.AddTransitionBarrier(Resource, Resource->GetResourceState(), after, D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES);
				Resource->SetResourceState(after);
			}
		}
		else
		{
			bool bWholeResourceWasTransitionedToSameState = true;

			for (CViewSubresourceSubset::CViewSubresourceIterator it = subresourceSubset.begin(); it != subresourceSubset.end(); ++it)
			{
				for (uint32 SubresourceIndex = it.StartSubresource(); SubresourceIndex < it.EndSubresource(); SubresourceIndex++)
				{
					if (Resource->IsTransitionNeeded(after))
					{
						hCommandList.AddTransitionBarrier(Resource, Resource->GetResourceState(), after, SubresourceIndex);
						Resource->SetResourceState(after);
					}
				}
			}

			// If we just transtioned every subresource to the same state, lets update it's tracking so it's on a per-resource level
			if (bWholeResourceWasTransitionedToSameState)
			{
				Resource->SetResourceState(after);
			}
		}
	}

	inline void TransitionResource(CommandListHandle& pCommandList, DepthStencilView* View)
	{
		// Determine the required subresource states from the view desc
		const D3D12_DEPTH_STENCIL_VIEW_DESC& DSVDesc = View->GetDesc();
		const bool bDSVDepthIsWritable = (DSVDesc.Flags & D3D12_DSV_FLAG_READ_ONLY_DEPTH) == 0;
		const bool bDSVStencilIsWritable = (DSVDesc.Flags & D3D12_DSV_FLAG_READ_ONLY_STENCIL) == 0;
		// TODO: Check if the PSO depth stencil is writable. When this is done, we need to transition in SetDepthStencilState too.

		// This code assumes that the DSV always contains the depth plane
		const bool bHasDepth = true;
		const bool bHasStencil = View->HasStencil();
		const bool bDepthIsWritable = bHasDepth && bDSVDepthIsWritable;
		const bool bStencilIsWritable = bHasStencil && bDSVStencilIsWritable;

		// DEPTH_WRITE is suitable for read operations when used as a normal depth/stencil buffer.
		auto pResource = View->GetResourceLocation();
		if (bDepthIsWritable)
		{
			TransitionResource(pCommandList, pResource, D3D12_RESOURCE_STATE_DEPTH_WRITE, View->GetDepthOnlyViewSubresourceSubset());
		}

		if (bStencilIsWritable)
		{
			TransitionResource(pCommandList, pResource, D3D12_RESOURCE_STATE_DEPTH_WRITE, View->GetStencilOnlyViewSubresourceSubset());
		}
	}

	

	inline void TransitionResource(CommandListHandle& hCommandList, ResourceHolder* Resource, D3D12_RESOURCE_STATES after, uint32 subresource)
	{
		if (Resource->IsTransitionNeeded(after))
		{
			hCommandList.AddTransitionBarrier(Resource, Resource->GetResourceState(), after, subresource);
			Resource->SetResourceState(after);
		}
	}

	void TransitionResource(CommandListHandle& hCommandList, DepthStencilView* pView, D3D12_RESOURCE_STATES after);

	void TransitionResource(CommandListHandle& hCommandList, RenderTargetView* pView, D3D12_RESOURCE_STATES after);
}
