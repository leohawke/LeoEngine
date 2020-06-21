#include "SamplerState.h"
#include "NodeDevice.h"
#include <mutex>

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

std::mutex GD3D12SamplerStateCacheLock;

std::shared_ptr<SamplerState> NodeDevice::CreateSampler(const D3D12_SAMPLER_DESC& Desc)
{
	std::unique_lock Lock(GD3D12SamplerStateCacheLock);

	auto PreviouslyCreated = SamplerMap.find(Desc);

	if (PreviouslyCreated != SamplerMap.end())
		return PreviouslyCreated->second;
	else {
		// 16-bit IDs are used for faster hashing
		lconstraint(SamplerID < 0xffff);

		auto NewSampler =std::make_shared<SamplerState>(this, Desc, static_cast<uint16>(SamplerID));

		SamplerMap.emplace(Desc, NewSampler);

		SamplerID++;

		return NewSampler;
	}
}
