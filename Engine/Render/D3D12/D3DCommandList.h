#pragma once

#include <LBase/linttype.hpp>
#include "d3d12_dxgi.h"

namespace platform_ex::Windows::D3D12 {

	using namespace leo::inttype;

	class CommandAllocator
	{

	};

	class CommandListHandle
	{
	private:
		class CommandListData
		{};
	public:
		uint64 CurrentGeneration() const;
		
		void WaitForCompletion(uint64 Generation) const;

		bool IsComplete(uint64 Generation) const;

		ID3D12CommandList* CommandList() const;

		friend bool operator==(const CommandListHandle& lhs, std::nullptr_t);

		friend bool operator!=(const CommandListHandle& lhs, std::nullptr_t);
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
