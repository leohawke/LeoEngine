/*! \file Engine\Render\D3D12\Fence.h
\ingroup Render
\brief D3D12渲染同步简易包装。
*/

#ifndef LE_RENDER_D3D12_Fence_h
#define LE_RENDER_D3D12_Fence_h 1

#include <LBase/linttype.hpp>
#include <atomic>
#include "d3d12_dxgi.h"
#include "../../Win32/NTHandle.h"


namespace platform_ex::Windows::D3D12 {
	class Fence {
	public:
		enum Type
		{
			Render,
			Compute,
			Copy
		};

		Fence();
		~Fence();

		uint64 Signal(Type type);
		void Wait(uint64 id);
		bool Completed(uint64 id);

		uint64 GetLastCompletedFenceFast() const { return last_completed_val; };

		uint64 GetCurrentFence() const { return fence_val; }
	private:
		COMPtr<ID3D12Fence> fence;
		UniqueNtHandle fence_event;
		uint64 last_completed_val;
		std::atomic<uint64> fence_val;
	};

	Fence& GetRenderFence();

	class CLSyncPoint
	{
	public:

		CLSyncPoint() : Generation(0) {}

		CLSyncPoint(Fence& CL) : RenderFence(&CL), Generation(CL.GetCurrentFence()) {}

		CLSyncPoint(const CLSyncPoint& SyncPoint) : RenderFence(SyncPoint.RenderFence), Generation(SyncPoint.Generation) {}

		CLSyncPoint& operator = (Fence& CL)
		{
			RenderFence = &CL;
			Generation = CL.GetCurrentFence();

			return *this;
		}

		CLSyncPoint& operator = (const CLSyncPoint& SyncPoint)
		{
			RenderFence = SyncPoint.RenderFence;
			Generation = SyncPoint.Generation;

			return *this;
		}

		bool operator!() const
		{
			return RenderFence == nullptr;
		}

		bool IsValid() const
		{
			return RenderFence != nullptr;
		}

		bool IsOpen() const
		{
			return Generation == RenderFence->GetCurrentFence();
		}

		bool IsComplete() const
		{
			return RenderFence->Completed(Generation);
		}

		void WaitForCompletion() const
		{
			RenderFence->Wait(Generation);
		}

		uint64 GetGeneration() const
		{
			return Generation;
		}
	private:
		Fence* RenderFence = nullptr;
		uint64                  Generation;
	};
}

#endif