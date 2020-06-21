#include "SamplerState.h"
#include "NodeDevice.h"

using namespace platform_ex::Windows::D3D12;

SamplerState::SamplerState(NodeDevice* InParent, const D3D12_SAMPLER_DESC& Desc, uint16 SamplerID)
	:DeviceChild(InParent)
	, ID(SamplerID)
{
	Descriptor.ptr = 0;
	auto& DescriptorAllocator = GetParentDevice()->GetSamplerDescriptorAllocator();
	Descriptor = DescriptorAllocator.AllocateHeapSlot(DescriptorHeapIndex);

	GetParentDevice()->CreateSamplerInternal(Desc, Descriptor);
}

SamplerState::~SamplerState()
{
	if (Descriptor.ptr)
	{
		auto& DescriptorAllocator = GetParentDevice()->GetSamplerDescriptorAllocator();
		DescriptorAllocator.FreeHeapSlot(Descriptor, DescriptorHeapIndex);
		Descriptor.ptr = 0;
	}
}
