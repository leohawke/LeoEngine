#include "NodeDevice.h"
#include "CommandContext.h"
#include "Context.h"

extern int GGlobalViewHeapSize;
using namespace platform_ex::Windows::D3D12;

NodeDevice::NodeDevice(GPUMaskType InGPUMask, D3D12Adapter* InAdapter)
	:SingleNodeGPUObject(InGPUMask),AdapterChild(InAdapter),
	RTVAllocator(InGPUMask, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 256),
	DSVAllocator(InGPUMask, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 256),
	SRVAllocator(InGPUMask, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 1024),
	UAVAllocator(InGPUMask, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 1024),
	SamplerAllocator(InGPUMask, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, 128)
	, GlobalSamplerHeap(this, InGPUMask)
	,GlobalViewHeap(this, InGPUMask)
{
}

void NodeDevice::Initialize()
{
	SetupAfterDeviceCreation();
}

ID3D12Device* NodeDevice::GetDevice()
{
	return GetParentAdapter()->GetDevice();
}

void NodeDevice::SetupAfterDeviceCreation()
{
	ID3D12Device* Direct3DDevice = GetParentAdapter()->GetDevice();

	GlobalSamplerHeap.Init(NUM_SAMPLER_DESCRIPTORS, D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);

	// This value can be tuned on a per app basis. I.e. most apps will never run into descriptor heap pressure so
	// can make this global heap smaller
	uint32 NumGlobalViewDesc = GGlobalViewHeapSize;

	const D3D12_RESOURCE_BINDING_TIER Tier = GetParentAdapter()->GetResourceBindingTier();
	uint32 MaximumSupportedHeapSize = NUM_VIEW_DESCRIPTORS_TIER_1;

	switch (Tier)
	{
	case D3D12_RESOURCE_BINDING_TIER_1:
		MaximumSupportedHeapSize = NUM_VIEW_DESCRIPTORS_TIER_1;
		break;
	case D3D12_RESOURCE_BINDING_TIER_2:
		MaximumSupportedHeapSize = NUM_VIEW_DESCRIPTORS_TIER_2;
		break;
	case D3D12_RESOURCE_BINDING_TIER_3:
	default:
		MaximumSupportedHeapSize = NUM_VIEW_DESCRIPTORS_TIER_3;
		break;
	}
	lconstraint(NumGlobalViewDesc <= MaximumSupportedHeapSize);

	// Init offline descriptor allocators
	RTVAllocator.Init(Direct3DDevice);
	DSVAllocator.Init(Direct3DDevice);
	SRVAllocator.Init(Direct3DDevice);
	UAVAllocator.Init(Direct3DDevice);
	SamplerAllocator.Init(Direct3DDevice);

	GlobalViewHeap.Init(NumGlobalViewDesc, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	CreateCommandContexts();
}

void NodeDevice::CreateCommandContexts()
{
	const uint32 NumContexts = 1;
	const uint32 NumAsyncComputeContexts = 0;
	const uint32 TotalContexts = NumContexts + NumAsyncComputeContexts;

	const uint32 DescriptorSuballocationPerContext = GlobalViewHeap.GetTotalSize() / TotalContexts;
	uint32 CurrentGlobalHeapOffset = 0;

	for (uint32 i = 0; i < NumContexts; ++i)
	{
		SubAllocatedOnlineHeap::SubAllocationDesc SubHeapDesc(&GlobalViewHeap, CurrentGlobalHeapOffset, DescriptorSuballocationPerContext);

		const bool bIsDefaultContext = (i == 0);

		auto NewCmdContext = new CommandContext(this, SubHeapDesc, bIsDefaultContext);

		CurrentGlobalHeapOffset += DescriptorSuballocationPerContext;

		// without that the first RHIClear would get a scissor rect of (0,0)-(0,0) which means we get a draw call clear 
		NewCmdContext->SetScissorRect(false, 0, 0, 0, 0);

		CommandContextArray.push_back(NewCmdContext);
	}

	CommandContextArray[0]->OpenCommandList();
}
