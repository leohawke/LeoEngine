#include "ConstantBuffer.h"
#include "Adapter.h"
#include "NodeDevice.h"
using namespace  platform_ex::Windows::D3D12;

ConstantBuffer::ConstantBuffer(NodeDevice* InParent, FastConstantAllocator& Allocator)
    :DeviceChild(InParent)
{
    Buffer.reset(InParent->GetParentAdapter()->CreateConstanBuffer(platform::Render::Buffer::Usage::Dynamic, 0, MAX_GLOBAL_CONSTANT_BUFFER_SIZE, EFormat::EF_Unknown));
}

bool ConstantBuffer::Version(ID3D12Resource*& BufferOut, bool bDiscardSharedConstants)
{
	// If nothing has changed there is no need to alloc a new buffer.
	if (CurrentUpdateSize == 0)
	{
		return false;
	}

	//SCOPE_CYCLE_COUNTER(STAT_D3D12GlobalConstantBufferUpdateTime);

	if (bDiscardSharedConstants)
	{
		// If we're discarding shared constants, just use constants that have been updated since the last Commit.
		TotalUpdateSize = CurrentUpdateSize;
	}
	else
	{
		// If we're re-using shared constants, use all constants.
		TotalUpdateSize = std::max(CurrentUpdateSize, TotalUpdateSize);
	}

	Buffer->UpdateSubresource(0, TotalUpdateSize, ShadowData);

	bIsDirty = false;

	BufferOut = Buffer->Resource();

    return true;
}
