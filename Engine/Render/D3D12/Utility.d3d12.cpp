#include "Utility.h"
#include "Fence.h"

using namespace platform_ex::Windows::D3D12;

bool SyncPoint::IsValid() const
{
    return Fence != nullptr;
}

bool SyncPoint::IsComplete() const
{
    return Fence->IsFenceComplete(Value);
}

void platform_ex::Windows::D3D12::SyncPoint::WaitForCompletion() const
{
    Fence->WaitForFence(Value);
}
