#include "Fence.h"
#include "Context.h"
#include <Windows.h>

namespace platform_ex::Windows::D3D12 {
	Fence::Fence()
		:last_completed_val(0),fence_val(1),fence_event(::CreateEventExW(nullptr,nullptr,0,EVENT_ALL_ACCESS))
	{
		Context::Instance().GetDevice()->CreateFence(0, D3D12_FENCE_FLAG_NONE, COMPtr_RefParam(fence,IID_ID3D12Fence));
	}
	Fence::~Fence() = default;

	uint64 Fence::Signal(Type type)
	{
		uint64 const id = fence_val;
		Context::Instance().GetCommandQueue((Device::CommandType)type)->Signal(fence.Get(),id);
		++fence_val;
		return id;
	}
	void Fence::Wait(uint64 id)
	{
		if (!Completed(id)) {
			fence->SetEventOnCompletion(id, fence_event.Get());
			::WaitForSingleObjectEx(fence_event.Get(), INFINITE, FALSE);
		}
	}
	bool Fence::Completed(uint64 id)
	{
		if (id > last_completed_val) {
			last_completed_val = std::max(last_completed_val, fence->GetCompletedValue());
		}
		return id <= last_completed_val;
	}
}
