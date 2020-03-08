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
	private:
		COMPtr<ID3D12Fence> fence;
		UniqueNtHandle fence_event;
		uint64 last_completed_val;
		std::atomic<uint64> fence_val;
	};
}

#endif