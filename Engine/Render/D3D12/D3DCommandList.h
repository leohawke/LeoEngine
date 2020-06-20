#pragma once

#include "d3d12_dxgi.h"
#include "Utility.h"
#include <atomic>

namespace platform_ex::Windows::D3D12 {
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
		class CommandListData :public DeviceChild,public SingleNodeGPUObject
		{
		public:
			CommandListData(NodeDevice* ParentDevice, D3D12_COMMAND_LIST_TYPE InCommandListType, CommandAllocator& CommandAllocator, CommandListManager* InCommandListManager);

			~CommandListData();

			void Close();

			bool IsComplete(uint64 Generation);

			mutable std::atomic<uint32>	NumRefs;

			CommandListManager* CommandListManager;
			CommandContext* CurrentOwningContext;
			const D3D12_COMMAND_LIST_TYPE			CommandListType;
			COMPtr<ID3D12GraphicsCommandList>	CommandList;		// Raw D3D command list pointer
			COMPtr<ID3D12GraphicsCommandList1>	CommandList1;
			COMPtr<ID3D12GraphicsCommandList2>	CommandList2;

			CommandAllocator* CurrentCommandAllocator;	// Command allocator currently being used for recording the command list

			uint64									CurrentGeneration;
			uint64									LastCompleteGeneration;

			bool									IsClosed;
		};
	public:
		uint64 CurrentGeneration() const;
		
		void WaitForCompletion(uint64 Generation) const;

		bool IsComplete(uint64 Generation) const;

		ID3D12CommandList* CommandList() const;

		friend bool operator==(const CommandListHandle& lhs, std::nullptr_t);

		friend bool operator!=(const CommandListHandle& lhs, std::nullptr_t);

		void SetCurrentOwningContext(CommandContext* context);

		void Close();

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

}
