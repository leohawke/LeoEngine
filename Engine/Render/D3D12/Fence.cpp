#include "Fence.h"
#include "NodeDevice.h"
#include "Adapter.h"
#include <Windows.h>

using namespace platform_ex::Windows;
using namespace platform_ex::Windows::D3D12;

FenceCore::FenceCore(D3D12Adapter* InParent, uint64 InitialValue, uint32 GPUIndex)
	:AdapterChild(InParent)
	, FenceValueAvailableAt(0)
	, GPUIndex(GPUIndex)
	, hFenceCompleteEvent(INVALID_HANDLE_VALUE)
{
	lconstraint(InParent);

	hFenceCompleteEvent = CreateEvent(nullptr, false, false, nullptr);
	lconstraint(INVALID_HANDLE_VALUE != hFenceCompleteEvent);

	CheckHResult(InParent->GetDevice()->CreateFence(InitialValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&Fence.ReleaseAndGetRef())));
}
FenceCore::~FenceCore()
{
	if (hFenceCompleteEvent != INVALID_HANDLE_VALUE)
	{
		CloseHandle(hFenceCompleteEvent);
		hFenceCompleteEvent = INVALID_HANDLE_VALUE;
	}
}

Fence::Fence(D3D12Adapter* InParent, GPUMaskType InGpuMask, const std::string& InName)
	:AdapterChild(InParent)
	, MultiNodeGPUObject(InGpuMask, InGpuMask)
	,
	last_completed_val(0), fence_val(0)
{
	std::memset(FenceCores, 0, sizeof(FenceCores));
}
Fence::~Fence()
{
	Destroy();
}

void Fence::Destroy()
{
	for (auto& FenceCore : FenceCores)
	{
		delete FenceCore;
		FenceCore = nullptr;
	}
}

void Fence::CreateFence()
{
	const uint32 GPUIndex = GetGPUMask();

	auto FenceCore = new D3D12::FenceCore(GetParentAdapter(), 0, GPUIndex);

	FenceCores[GPUIndex] = FenceCore;

	last_completed_val = 0;
	fence_val = last_completed_val + 1;
}

uint64 Fence::Signal(CommandQueueType type)
{
	uint64 id = fence_val;

	InternalSignal(type, fence_val);

	UpdateLastCompletedFence();

	++fence_val;
	return id;
}

void Fence::GpuWait(uint32 DeviceIndex, CommandQueueType InQueueType, uint64 FenceValue, uint32 FenceGPUIndex)
{
	auto CommandQueue = GetParentAdapter()->GetNodeDevice(DeviceIndex)->GetD3DCommandQueue(InQueueType);

	auto FenceCore = FenceCores[FenceGPUIndex];

	CheckHResult(CommandQueue->Wait(FenceCore->GetFence(), FenceValue));
}

bool Fence::IsFenceComplete(uint64 FenceValue)
{
	if (FenceValue < last_completed_val)
	{
		return true;
	}

	return FenceValue <= UpdateLastCompletedFence();
}

uint64 Fence::UpdateLastCompletedFence()
{
	uint64 CompletedFence = MAXUINT64;

	auto GPUIndex = GetGPUMask();
	auto FenceCore = FenceCores[GPUIndex];
	lconstraint(FenceCore);
	auto completed_value = FenceCore->GetFence()->GetCompletedValue();
	CompletedFence = std::min<uint64>(completed_value, CompletedFence);

	// Must be computed on the stack because the function can be called concurrently.
	last_completed_val = CompletedFence;
	return CompletedFence;
}

void Fence::InternalSignal(CommandQueueType InQueueType, uint64 FenceToSignal)
{
	auto GPUIndex = GetGPUMask();

	auto CommandQueue = GetParentAdapter()->GetNodeDevice(GPUIndex)->GetD3DCommandQueue(InQueueType);
	lconstraint(CommandQueue);
	auto FenceCore = FenceCores[GPUIndex];
	lconstraint(FenceCore);

	CommandQueue->Signal(FenceCore->GetFence(), FenceToSignal);
}

void Fence::WaitForFence(uint64 FenceValue)
{
	if (!IsFenceComplete(FenceValue))
	{
		auto GPUIndex = GetGPUMask();
		auto FenceCore = FenceCores[GPUIndex];

		if (FenceValue > FenceCore->GetFence()->GetCompletedValue())
		{
			CheckHResult(FenceCore->GetFence()->SetEventOnCompletion(FenceValue, FenceCore->GetCompletionEvent()));

			// Wait for the event to complete (the event is automatically reset afterwards)
			const uint32 WaitResult = WaitForSingleObject(FenceCore->GetCompletionEvent(), INFINITE);
		}

		UpdateLastCompletedFence();
	}
}


