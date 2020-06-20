#pragma once

#include <LBase/linttype.hpp>
#include "d3d12_dxgi.h"


namespace platform_ex::Windows::D3D12 {
	class Fence;

	class SyncPoint
	{
	public:
		explicit SyncPoint()
			: Fence(nullptr)
			, Value(0)
		{
		}

		explicit SyncPoint(Fence* InFence, uint64 InValue)
			: Fence(InFence)
			, Value(InValue)
		{
		}

		bool IsValid() const;
		bool IsComplete() const;
		void WaitForCompletion() const;

	private:
		Fence* Fence;
		uint64 Value;
	};
}