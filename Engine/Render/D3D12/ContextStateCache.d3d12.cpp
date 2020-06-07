#include "CommandContext.h"
#include <LBase/lmemory.hpp>
using namespace platform_ex::Windows::D3D12;

void CommandContextStateCache::SetBlendFactor(const float BlendFactor[4])
{
	if (std::memcmp(PipelineState.Graphics.CurrentBlendFactor, BlendFactor, sizeof(PipelineState.Graphics.CurrentBlendFactor)))
	{
		std::memcpy(PipelineState.Graphics.CurrentBlendFactor, BlendFactor, sizeof(PipelineState.Graphics.CurrentBlendFactor));
		bNeedSetBlendFactor = true;
	}
}

void CommandContextStateCache::SetStencilRef(uint32 StencilRef)
{
	if (PipelineState.Graphics.CurrentReferenceStencil != StencilRef)
	{
		PipelineState.Graphics.CurrentReferenceStencil = StencilRef;
		bNeedSetStencilRef = true;
	}
}

void CommandContextStateCache::InternalSetIndexBuffer(GraphicsBuffer* IndexBufferLocation, DXGI_FORMAT Format, uint32 Offset)
{
	auto& CommandList = CmdContext->CommandListHandle;

	CommandList->IASetIndexBuffer(&PipelineState.Graphics.IBCache.CurrentIndexBufferView);

	//ResourceBarrier
}

static uint32 FloorLog2(uint32 Value)
{
	unsigned long Log2;
	if (_BitScanReverse(&Log2, Value) != 0)
	{
		return Log2;
	}

	return 0;
}

void CommandContextStateCache::InternalSetStreamSource(GraphicsBuffer* VertexBufferLocation, uint32 StreamIndex, uint32 Stride, uint32 Offset)
{
	__declspec(align(16)) D3D12_VERTEX_BUFFER_VIEW NewView;
	NewView.BufferLocation = (VertexBufferLocation) ? VertexBufferLocation->GetGPUVirtualAddress() + Offset : 0;
	NewView.StrideInBytes = Stride;
	NewView.SizeInBytes = (VertexBufferLocation) ? VertexBufferLocation->GetSize() - Offset : 0; // Make sure we account for how much we offset into the VB

	D3D12_VERTEX_BUFFER_VIEW& CurrentView = PipelineState.Graphics.VBCache.CurrentVertexBufferViews[StreamIndex];

	if (NewView.BufferLocation != CurrentView.BufferLocation ||
		NewView.StrideInBytes != CurrentView.StrideInBytes ||
		NewView.SizeInBytes != CurrentView.SizeInBytes)
	{
		bNeedSetVB = true;
		PipelineState.Graphics.VBCache.CurrentVertexBufferResources[StreamIndex] = VertexBufferLocation;

		if (VertexBufferLocation != nullptr)
		{
			std::memcpy(&CurrentView, &NewView, sizeof(CurrentView));
			PipelineState.Graphics.VBCache.BoundVBMask |= ((VBSlotMask)1 << StreamIndex);
		}
		else
		{
			std::memset(&CurrentView,0, sizeof(CurrentView));
			PipelineState.Graphics.VBCache.CurrentVertexBufferResources[StreamIndex] = nullptr;

			PipelineState.Graphics.VBCache.BoundVBMask &= ~((VBSlotMask)1 << StreamIndex);
		}

		if (PipelineState.Graphics.VBCache.BoundVBMask)
		{
			PipelineState.Graphics.VBCache.MaxBoundVertexBufferIndex = FloorLog2(PipelineState.Graphics.VBCache.BoundVBMask);
		}
		else
		{
			PipelineState.Graphics.VBCache.MaxBoundVertexBufferIndex = -1;
		}
	}

	//ResourceBarrier
}

void CommandContextStateCache::ClearSRVs()
{
	if (bSRVSCleared)
	{
		return;
	}

	PipelineState.Common.SRVCache.Clear();

	bSRVSCleared = true;
}

void CommandContextStateCache::Clear()
{
	ClearState();

	DescriptorCache.Clear();
}

void CommandContextStateCache::ClearState()
{
	// Shader Resource View State Cache
	bSRVSCleared = false;
	ClearSRVs();

	PipelineState.Common.CBVCache.Clear();
	PipelineState.Common.UAVCache.Clear();
	PipelineState.Common.SamplerCache.Clear();

	leo::memset(PipelineState.Common.CurrentShaderSamplerCounts,0);
	leo::memset(PipelineState.Common.CurrentShaderSRVCounts, 0);
	leo::memset(PipelineState.Common.CurrentShaderCBCounts, 0);
	leo::memset(PipelineState.Common.CurrentShaderUAVCounts, 0);

	PipelineState.Graphics.CurrentNumberOfStreamOutTargets = 0;
	PipelineState.Graphics.CurrentNumberOfScissorRects = 0;

	// Depth Stencil State Cache
	PipelineState.Graphics.CurrentReferenceStencil = D3D12_DEFAULT_STENCIL_REFERENCE;
	PipelineState.Graphics.CurrentDepthStencilTarget = nullptr;

	// Blend State Cache
	PipelineState.Graphics.CurrentBlendFactor[0] = D3D12_DEFAULT_BLEND_FACTOR_RED;
	PipelineState.Graphics.CurrentBlendFactor[1] = D3D12_DEFAULT_BLEND_FACTOR_GREEN;
	PipelineState.Graphics.CurrentBlendFactor[2] = D3D12_DEFAULT_BLEND_FACTOR_BLUE;
	PipelineState.Graphics.CurrentBlendFactor[3] = D3D12_DEFAULT_BLEND_FACTOR_ALPHA;

	leo::memset(PipelineState.Graphics.CurrentViewport, 0);
	PipelineState.Graphics.CurrentNumberOfViewports = 0;

	leo::memset(PipelineState.Graphics.CurrentScissorRects, 0);
	PipelineState.Graphics.CurrentNumberOfScissorRects = 0;

	PipelineState.Graphics.CurrentPipelineStateObject = nullptr;
	PipelineState.Common.CurrentPipelineStateObject = nullptr;

	leo::memset(PipelineState.Graphics.CurrentStreamOutTargets, 0);
	leo::memset(PipelineState.Graphics.CurrentSOOffsets,0);

	PipelineState.Graphics.VBCache.Clear();
	PipelineState.Graphics.IBCache.Clear();

	leo::memset(PipelineState.Graphics.RenderTargetArray, 0);
	PipelineState.Graphics.CurrentNumberOfRenderTargets = 0;

	PipelineState.Graphics.CurrentPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
	PipelineState.Graphics.CurrentPrimitiveType = platform::Render::PrimtivteType::ControlPoint_32;

	PipelineState.Graphics.MinDepth = 0.0f;
	PipelineState.Graphics.MaxDepth = 1.0f;
}

static void ValidateScissorRect(const D3D12_VIEWPORT& Viewport, const D3D12_RECT& ScissorRect)
{
	lconstraint(ScissorRect.left >= (LONG)Viewport.TopLeftX);
	lconstraint(ScissorRect.top >= (LONG)Viewport.TopLeftY);
	lconstraint(ScissorRect.right <= (LONG)Viewport.TopLeftX + (LONG)Viewport.Width);
	lconstraint(ScissorRect.bottom <= (LONG)Viewport.TopLeftY + (LONG)Viewport.Height);
	lconstraint(ScissorRect.left <= ScissorRect.right && ScissorRect.top <= ScissorRect.bottom);
}

void CommandContextStateCache::SetScissorRects(uint32 Count, const D3D12_RECT* const ScissorRects)
{
	lconstraint(Count < std::size(PipelineState.Graphics.CurrentScissorRects));

	for (uint32 Rect = 0; Rect < Count; ++Rect)
	{
		ValidateScissorRect(PipelineState.Graphics.CurrentViewport[Rect], ScissorRects[Rect]);
	}

	if ((PipelineState.Graphics.CurrentNumberOfScissorRects != Count || std::memcmp(&PipelineState.Graphics.CurrentScissorRects[0], ScissorRects, sizeof(D3D12_RECT) * Count)))
	{
		std::memcpy(&PipelineState.Graphics.CurrentScissorRects[0], ScissorRects, sizeof(D3D12_RECT) * Count);
		PipelineState.Graphics.CurrentNumberOfScissorRects = Count;
		bNeedSetScissorRects = true;
	}
}

void CommandContextStateCache::SetScissorRect(const D3D12_RECT& ScissorRect)
{
	ValidateScissorRect(PipelineState.Graphics.CurrentViewport[0], ScissorRect);

	if ((PipelineState.Graphics.CurrentNumberOfScissorRects != 1 || std::memcmp(&PipelineState.Graphics.CurrentScissorRects[0], &ScissorRect, sizeof(D3D12_RECT))))
	{
		std::memcpy(&PipelineState.Graphics.CurrentScissorRects[0], &ScissorRect, sizeof(D3D12_RECT));
		PipelineState.Graphics.CurrentNumberOfScissorRects = 1;
		bNeedSetScissorRects = true;
	}
}

void CommandContextStateCache::SetViewport(const D3D12_VIEWPORT& Viewport)
{
	if ((PipelineState.Graphics.CurrentNumberOfViewports != 1 || std::memcmp(&PipelineState.Graphics.CurrentViewport[0], &Viewport, sizeof(D3D12_VIEWPORT))))
	{
		std::memcpy(&PipelineState.Graphics.CurrentViewport[0], &Viewport, sizeof(D3D12_VIEWPORT));
		PipelineState.Graphics.CurrentNumberOfViewports = 1;
		bNeedSetViewports = true;
	}
}

void CommandContextStateCache::SetViewports(uint32 Count, const D3D12_VIEWPORT* const Viewports)
{
	lconstraint(Count < std::size(PipelineState.Graphics.CurrentViewport));
	if ((PipelineState.Graphics.CurrentNumberOfViewports != Count || std::memcpy(&PipelineState.Graphics.CurrentViewport[0], Viewports, sizeof(D3D12_VIEWPORT) * Count)))
	{
		std::memcpy(&PipelineState.Graphics.CurrentViewport[0], Viewports, sizeof(D3D12_VIEWPORT) * Count);
		PipelineState.Graphics.CurrentNumberOfViewports = Count;
		bNeedSetViewports = true;
	}
}

template<ShaderType ShaderFrequency>
inline void CommandContextStateCache::SetShaderResourceView(ShaderResourceView* SRV, uint32 ResourceIndex)
{
	lconstraint(ResourceIndex < MAX_SRVS);
	auto& Cache = PipelineState.Common.SRVCache;
	auto& CurrentShaderResourceViews = Cache.Views[ShaderFrequency];

	if ((CurrentShaderResourceViews[ResourceIndex] != SRV))
	{
		if (SRV != nullptr)
		{
			// Mark the SRVs as not cleared
			bSRVSCleared = false;

			Cache.BoundMask[ShaderFrequency] |= ((SRVSlotMask)1 << ResourceIndex);
		}
		else
		{
			Cache.BoundMask[ShaderFrequency] &= ~((SRVSlotMask)1 << ResourceIndex);
		}

		// Find the highest set SRV
		(Cache.BoundMask[ShaderFrequency] == 0) ? Cache.MaxBoundIndex[ShaderFrequency] = -1 :
			Cache.MaxBoundIndex[ShaderFrequency] = FMath::FloorLog2(Cache.BoundMask[ShaderFrequency]);

		CurrentShaderResourceViews[ResourceIndex] = SRV;
		ShaderResourceViewCache::DirtySlot(Cache.DirtySlotMask[ShaderFrequency], ResourceIndex);
	}
}

void CommandContextStateCache::SetRenderTargets(uint32 NumSimultaneousRenderTargets, const RenderTargetView* const* RTArray, const DepthStencilView* DSTarget)
{
	// Note: We assume that the have been checks to make sure this function is only called when there really are changes being made.
	// We always set descriptors after calling this function.
	bNeedSetRTs = true;

	// Update the depth stencil
	PipelineState.Graphics.CurrentDepthStencilTarget =const_cast<DepthStencilView*>(DSTarget);

	// Update the render targets
	leo::memset(PipelineState.Graphics.RenderTargetArray, 0);
	std::memcpy(PipelineState.Graphics.RenderTargetArray, RTArray, sizeof(RenderTargetView*) * NumSimultaneousRenderTargets);

	// In D3D11, the NumSimultaneousRenderTargets count was used even when setting RTV slots to null (to unbind them)
	// In D3D12, we don't do this. So we need change the count to match the non null views used.
	uint32 ActiveNumSimultaneousRenderTargets = 0;
	for (uint32 i = 0; i < NumSimultaneousRenderTargets; i++)
	{
		if (RTArray[i] != nullptr)
		{
			ActiveNumSimultaneousRenderTargets = i + 1;
		}
	}
	PipelineState.Graphics.CurrentNumberOfRenderTargets = ActiveNumSimultaneousRenderTargets;
}

void CommandContextStateCache::ApplySamplers(const RootSignature* const pRootSignature, uint32 StartStage, uint32 EndStage)
{

}

void CommandContextStateCache::DirtyStateForNewCommandList()
{

}

void CommandContextStateCache::DirtyState()
{

}

void CommandContextStateCache::DirtyViewDescriptorTables()
{

}

void CommandContextStateCache::DirtySamplerDescriptorTables()
{

}

bool CommandContextStateCache::AssertResourceStates(CachePipelineType PipelineType)
{
#ifdef NDEBUG
	return true;
#else
	ID3D12CommandList* pCommandList = CmdContext->CommandListHandle;
	COMPtr<ID3D12DebugCommandList> pDebugCommandList;
	CheckHResult(pCommandList->QueryInterface(&pDebugCommandList.ReleaseAndGetRef()));

	//TODO

	return true;
#endif
}



template<ShaderType ShaderStage>
void CommandContextStateCache::SetUAVs(uint32 UAVStartSlot, uint32 NumSimultaneousUAVs, UnorderedAccessView* const* UAVArray, uint32* UAVInitialCountArray)
{
	lconstraint(NumSimultaneousUAVs > 0);

	auto& Cache = PipelineState.Common.UAVCache;

	// When setting UAV's for Graphics, it wipes out all existing bound resources.
	const bool bIsCompute = ShaderStage == ShaderType::ComputeShader;
	Cache.StartSlot[ShaderStage] = bIsCompute ? std::min(UAVStartSlot, Cache.StartSlot[ShaderStage]) : 0;

	for (uint32 i = 0; i < NumSimultaneousUAVs; ++i)
	{
		auto* UAV = UAVArray[i];

		Cache.Views[ShaderStage][UAVStartSlot + i] = UAV;
		UnorderedAccessViewCache::DirtySlot(Cache.DirtySlotMask[ShaderStage], UAVStartSlot + i);

		if (UAV)
		{
			//TODO CounterResource
			if (UAV->CounterResource && (!UAV->CounterResourceInitialized || UAVInitialCountArray[i] != -1))
			{
			}
		}
		else
		{
		}
	}
}

template<ShaderType ShaderStage>
void CommandContextStateCache::ClearUAVs()
{
	auto& Cache = PipelineState.Common.UAVCache;
	const bool bIsCompute = ShaderStage == ShaderType::ComputeShader;

	for (uint32 i = 0; i < MAX_UAVS; ++i)
	{
		if (Cache.Views[ShaderStage][i] != nullptr)
		{
			UnorderedAccessViewCache::DirtySlot(Cache.DirtySlotMask[ShaderStage], i);
		}
		Cache.Views[ShaderStage][i] = nullptr;
	}
}

template<CachePipelineType PipelineType>
void CommandContextStateCache::ApplyState()
{
}



template<> void CommandContextStateCache::SetShaderResourceView<ShaderType::VertexShader>(ShaderResourceView* SRV, uint32 ResourceIndex);
template<> void CommandContextStateCache::SetShaderResourceView<ShaderType::PixelShader>(ShaderResourceView* SRV, uint32 ResourceIndex);

template<> void CommandContextStateCache::ApplyState<CPT_Graphics>();

template<> void CommandContextStateCache::SetUAVs<ShaderType::VertexShader>(uint32 UAVStartSlot, uint32 NumSimultaneousUAVs, UnorderedAccessView* const* UAVArray, uint32* UAVInitialCountArray);
template<> void CommandContextStateCache::SetUAVs<ShaderType::PixelShader>(uint32 UAVStartSlot, uint32 NumSimultaneousUAVs, UnorderedAccessView* const* UAVArray, uint32* UAVInitialCountArray);

template<> void CommandContextStateCache::ClearUAVs<ShaderType::VertexShader>();
template<> void CommandContextStateCache::ClearUAVs<ShaderType::PixelShader>();
