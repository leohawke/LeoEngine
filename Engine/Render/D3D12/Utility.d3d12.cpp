#include "Utility.h"
#include "Fence.h"
#include "HardwareShader.h"
#include "RootSignature.h"

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

void QuantizeBoundShaderState(const D3D12_RESOURCE_BINDING_TIER& ResourceBindingTier, const ComputeHWShader* const ComputeShader, QuantizedBoundShaderState& OutQBSS)
{
    std::memset(&OutQBSS, 0, sizeof(OutQBSS));

    QuantizedBoundShaderState::InitShaderRegisterCounts(ResourceBindingTier, ComputeShader->ResourceCounts, OutQBSS.RegisterCounts[ShaderType::VisibilityAll], true);
}
