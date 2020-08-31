#include <LBase/lmemory.hpp>
#include <LBase/linttype_utility.hpp>
#include <Engine/Win32/WindowsPlatformMath.h>
#include "NodeDevice.h"
#include "DescriptorCache.h"
#include "CommandContext.h"
#include "RootSignature.h"
#include "Adapter.h"
#include "SamplerState.h"

using namespace platform_ex::Windows::D3D12;
using leo::BitMask;

// This value defines how many descriptors will be in the device local view heap which
// This should be tweaked for each title as heaps require VRAM. The default value of 512k takes up ~16MB
int32 GLocalViewHeapSize = 500 * 1000;

// This value defines how many descriptors will be in the device global view heap which
// is shared across contexts to allow the driver to eliminate redundant descriptor heap sets.
// This should be tweaked for each title as heaps require VRAM. The default value of 512k takes up ~16MB
int32 GGlobalViewHeapSize = 500 * 1000;

CommandContextStateCache::CommandContextStateCache(GPUMaskType Node)
	:SingleNodeGPUObject(Node),
	DescriptorCache(Node)
{
}

void CommandContextStateCache::Init(NodeDevice* InParent, CommandContext* InCmdContext, const CommandContextStateCache* AncestralState, SubAllocatedOnlineHeap::SubAllocationDesc& SubHeapDesc)
{
	Parent = InParent;
	CmdContext = InCmdContext;

	// Cache the resource binding tier
	ResourceBindingTier = Parent->GetParentAdapter()->GetResourceBindingTier();

	// Init the descriptor heaps
	const uint32 MaxDescriptorsForTier = (ResourceBindingTier == D3D12_RESOURCE_BINDING_TIER_1) ? NUM_VIEW_DESCRIPTORS_TIER_1 :
		NUM_VIEW_DESCRIPTORS_TIER_2;

	lconstraint(GLocalViewHeapSize <= (int32)MaxDescriptorsForTier);
	lconstraint(GGlobalViewHeapSize <= (int32)MaxDescriptorsForTier);

	const uint32 NumSamplerDescriptors = NUM_SAMPLER_DESCRIPTORS;
	DescriptorCache.Init(InParent, InCmdContext, GLocalViewHeapSize, NumSamplerDescriptors, SubHeapDesc);

	if (AncestralState)
	{
		InheritState(*AncestralState);
	}
	else
	{
		ClearState();
	}
}

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

void CommandContextStateCache::SetComputeShader(ComputeHWShader* Shader)
{
	ComputeHWShader* CurrentShader = nullptr;
	GetComputeShader(&CurrentShader);
	if (CurrentShader != Shader)
	{
		// See if we need to change the root signature
		const auto* const pCurrentRootSignature = CurrentShader ? CurrentShader->pRootSignature : nullptr;
		const auto* const pNewRootSignature = Shader ? Shader->pRootSignature : nullptr;
		if (pCurrentRootSignature != pNewRootSignature)
		{
			PipelineState.Compute.bNeedSetRootSignature = true;
		}

		PipelineState.Common.CurrentShaderSamplerCounts[ShaderType::ComputeShader] = (Shader) ? Shader->ResourceCounts.NumSamplers : 0;
		PipelineState.Common.CurrentShaderSRVCounts[ShaderType::ComputeShader] = (Shader) ? Shader->ResourceCounts.NumSRVs : 0;
		PipelineState.Common.CurrentShaderCBCounts[ShaderType::ComputeShader] = (Shader) ? Shader->ResourceCounts.NumCBs : 0;
		PipelineState.Common.CurrentShaderUAVCounts[ShaderType::ComputeShader] = (Shader) ? Shader->ResourceCounts.NumUAVs : 0;

		// Shader changed so its resource table is dirty
		CmdContext->DirtyConstantBuffers[ShaderType::ComputeShader] = 0xffff;
	}
}

void CommandContextStateCache::InternalSetIndexBuffer(ResourceHolder* IndexBufferLocation, DXGI_FORMAT Format, uint32 Offset)
{
	auto& CommandList = CmdContext->CommandListHandle;

	CommandList->IASetIndexBuffer(&PipelineState.Graphics.IBCache.CurrentIndexBufferView);

	//ResourceBarrier
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

	PipelineState.Compute.ComputeBudget = ::platform::Render::AsyncComputeBudget::All_4;
	PipelineState.Graphics.CurrentPipelineStateObject = nullptr;
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
void CommandContextStateCache::SetShaderResourceView(ShaderResourceView* SRV, uint32 ResourceIndex)
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
			Cache.MaxBoundIndex[ShaderFrequency] = FloorLog2(Cache.BoundMask[ShaderFrequency]);

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
	bool HighLevelCacheMiss = false;

	auto& Cache = PipelineState.Common.SamplerCache;
	SamplerSlotMask CurrentShaderDirtySamplerSlots[ShaderType::NumStandardType] = {};
	uint32 NumSamplers[ShaderType::NumStandardType + 1] = {};

	const auto& pfnCalcSamplersNeeded = [&]()
	{
		NumSamplers[ShaderType::NumStandardType] = 0;

		for (uint32 Stage = StartStage; Stage < EndStage; ++Stage)
		{
			// Note this code assumes the starting register is index 0.
			const SamplerSlotMask CurrentShaderSamplerRegisterMask = ((SamplerSlotMask)1 << PipelineState.Common.CurrentShaderSamplerCounts[Stage]) - (SamplerSlotMask)1;
			CurrentShaderDirtySamplerSlots[Stage] = CurrentShaderSamplerRegisterMask & Cache.DirtySlotMask[Stage];
			if (CurrentShaderDirtySamplerSlots[Stage])
			{
				if (ResourceBindingTier == D3D12_RESOURCE_BINDING_TIER_1)
				{
					// Tier 1 HW requires the full number of sampler descriptors defined in the root signature.
					NumSamplers[Stage] = pRootSignature->MaxSamplerCount(Stage);
				}
				else
				{
					NumSamplers[Stage] = PipelineState.Common.CurrentShaderSamplerCounts[Stage];
				}

				lconstraint(NumSamplers[Stage] > 0 && NumSamplers[Stage] <= MAX_SAMPLERS);
				NumSamplers[ShaderType::NumStandardType] += NumSamplers[Stage];
			}
		}
	};

	pfnCalcSamplersNeeded();

	if (DescriptorCache.UsingGlobalSamplerHeap())
	{
		auto& GlobalSamplerSet = DescriptorCache.GetLocalSamplerSet();
		auto& CommandList = CmdContext->CommandListHandle;

		for (uint32 Stage = StartStage; Stage < EndStage; Stage++)
		{
			if ((CurrentShaderDirtySamplerSlots[Stage] && NumSamplers[Stage]))
			{
				SamplerSlotMask& CurrentDirtySlotMask = Cache.DirtySlotMask[Stage];
				SamplerState** Samplers = Cache.States[Stage];

				UniqueSamplerTable Table;
				Table.Key.Count = NumSamplers[Stage];

				for (uint32 i = 0; i < NumSamplers[Stage]; i++)
				{
					Table.Key.SamplerID[i] = Samplers[i] ? Samplers[i]->ID : 0;
					SamplerStateCache::CleanSlot(CurrentDirtySlotMask, i);
				}

				auto CachedTableItr = GlobalSamplerSet.find(Table);
				const UniqueSamplerTable* CachedTable = CachedTableItr !=GlobalSamplerSet.end()? &(*CachedTableItr):nullptr;
				if (CachedTable)
				{
					// Make sure the global sampler heap is really set on the command list before we try to find a cached descriptor table for it.
					lconstraint(DescriptorCache.IsHeapSet(GetParentDevice()->GetGlobalSamplerHeap().GetHeap()));
					lconstraint(CachedTable->GPUHandle.ptr);
					if (Stage == ShaderType::ComputeShader)
					{
						const uint32 RDTIndex = pRootSignature->SamplerRDTBindSlot(ShaderType(Stage));
						CommandList->SetComputeRootDescriptorTable(RDTIndex, CachedTable->GPUHandle);
					}
					else
					{
						const uint32 RDTIndex = pRootSignature->SamplerRDTBindSlot(ShaderType(Stage));
						CommandList->SetGraphicsRootDescriptorTable(RDTIndex, CachedTable->GPUHandle);
					}

					// We changed the descriptor table, so all resources bound to slots outside of the table's range are now dirty.
					// If a shader needs to use resources bound to these slots later, we need to set the descriptor table again to ensure those
					// descriptors are valid.
					const SamplerSlotMask OutsideCurrentTableRegisterMask = ~(((SamplerSlotMask)1 << Table.Key.Count) - (SamplerSlotMask)1);
					Cache.Dirty(static_cast<ShaderType>(Stage), OutsideCurrentTableRegisterMask);
				}
				else
				{
					HighLevelCacheMiss = true;
					break;
				}
			}
		}

		if (!HighLevelCacheMiss)
		{
			// Success, all the tables were found in the high level heap
			return;
		}
	}

	if (HighLevelCacheMiss)
	{
		// Move to per context heap strategy
		const bool bDescriptorHeapsChanged = DescriptorCache.SwitchToContextLocalSamplerHeap();
		if (bDescriptorHeapsChanged)
		{
			// If descriptor heaps changed, then all our tables are dirty again and we need to recalculate the number of slots we need.
			pfnCalcSamplersNeeded();
		}
	}

	auto SamplerHeap = DescriptorCache.GetCurrentSamplerHeap();
	lconstraint(DescriptorCache.UsingGlobalSamplerHeap() == false);
	lconstraint(SamplerHeap != &GetParentDevice()->GetGlobalSamplerHeap());
	lconstraint(DescriptorCache.IsHeapSet(SamplerHeap->GetHeap()));
	lconstraint(!DescriptorCache.IsHeapSet(GetParentDevice()->GetGlobalSamplerHeap().GetHeap()));

	if (!SamplerHeap->CanReserveSlots(NumSamplers[ShaderType::NumStandardType]))
	{
		const bool bDescriptorHeapsChanged = SamplerHeap->RollOver();
		if (bDescriptorHeapsChanged)
		{
			// If descriptor heaps changed, then all our tables are dirty again and we need to recalculate the number of slots we need.
			pfnCalcSamplersNeeded();
		}
	}
	uint32 SamplerHeapSlot = SamplerHeap->ReserveSlots(NumSamplers[ShaderType::NumStandardType]);

#define CONDITIONAL_SET_SAMPLERS(Shader) \
	if (CurrentShaderDirtySamplerSlots[##Shader]) \
	{ \
		DescriptorCache.SetSamplers<##Shader>(pRootSignature, Cache, CurrentShaderDirtySamplerSlots[##Shader], NumSamplers[##Shader], SamplerHeapSlot); \
	}

	if (StartStage == ShaderType::ComputeShader)
	{
		CONDITIONAL_SET_SAMPLERS(ShaderType::ComputeShader);
	}
	else
	{
		CONDITIONAL_SET_SAMPLERS(ShaderType::VertexShader);
		CONDITIONAL_SET_SAMPLERS(ShaderType::HullShader);
		CONDITIONAL_SET_SAMPLERS(ShaderType::DomainShader);
		CONDITIONAL_SET_SAMPLERS(ShaderType::GeometryShader);
		CONDITIONAL_SET_SAMPLERS(ShaderType::PixelShader);
	}
#undef CONDITIONAL_SET_SAMPLERS

	SamplerHeap->SetNextSlot(SamplerHeapSlot);
}

void CommandContextStateCache::DirtyStateForNewCommandList()
{
	// Dirty state that doesn't align with command list defaults.

	// Always need to set PSOs and root signatures
	PipelineState.Common.bNeedSetPSO = true;
	PipelineState.Graphics.bNeedSetRootSignature = true;
	bNeedSetPrimitiveTopology = true;


	if (PipelineState.Graphics.VBCache.BoundVBMask) { bNeedSetVB = true; }

	// IndexBuffers are set in DrawIndexed*() calls, so there's no way to depend on previously set IndexBuffers without making a new DrawIndexed*() call.
	PipelineState.Graphics.IBCache.Clear();

	if (PipelineState.Graphics.CurrentNumberOfStreamOutTargets) { bNeedSetSOs = true; }
	if (PipelineState.Graphics.CurrentNumberOfRenderTargets || PipelineState.Graphics.CurrentDepthStencilTarget) { bNeedSetRTs = true; }
	if (PipelineState.Graphics.CurrentNumberOfViewports) { bNeedSetViewports = true; }
	if (PipelineState.Graphics.CurrentNumberOfScissorRects) { bNeedSetScissorRects = true; }

	if (PipelineState.Graphics.CurrentBlendFactor[0] != D3D12_DEFAULT_BLEND_FACTOR_RED ||
		PipelineState.Graphics.CurrentBlendFactor[1] != D3D12_DEFAULT_BLEND_FACTOR_GREEN ||
		PipelineState.Graphics.CurrentBlendFactor[2] != D3D12_DEFAULT_BLEND_FACTOR_BLUE ||
		PipelineState.Graphics.CurrentBlendFactor[3] != D3D12_DEFAULT_BLEND_FACTOR_ALPHA)
	{
		bNeedSetBlendFactor = true;
	}

	if (PipelineState.Graphics.CurrentReferenceStencil != D3D12_DEFAULT_STENCIL_REFERENCE) { bNeedSetStencilRef = true; }

	if (PipelineState.Graphics.MinDepth != 0.0 ||
		PipelineState.Graphics.MaxDepth != 1.0)
	{
		bNeedSetDepthBounds = false;
	}

	// Always dirty View and Sampler bindings. We detect the slots that are actually used at Draw/Dispatch time.
	PipelineState.Common.SRVCache.DirtyAll();
	PipelineState.Common.UAVCache.DirtyAll();
	PipelineState.Common.CBVCache.DirtyAll();
	PipelineState.Common.SamplerCache.DirtyAll();
}

void CommandContextStateCache::DirtyState()
{
	// Mark bits dirty so the next call to ApplyState will set all this state again
	PipelineState.Common.bNeedSetPSO = true;
	PipelineState.Graphics.bNeedSetRootSignature = true;
	bNeedSetVB = true;
	bNeedSetSOs = true;
	bNeedSetRTs = true;
	bNeedSetViewports = true;
	bNeedSetScissorRects = true;
	bNeedSetPrimitiveTopology = true;
	bNeedSetBlendFactor = true;
	bNeedSetStencilRef = true;
	bNeedSetDepthBounds = false;
	PipelineState.Common.SRVCache.DirtyAll();
	PipelineState.Common.UAVCache.DirtyAll();
	PipelineState.Common.CBVCache.DirtyAll();
	PipelineState.Common.SamplerCache.DirtyAll();
}

void CommandContextStateCache::DirtyViewDescriptorTables()
{
	// Mark the CBV/SRV/UAV descriptor tables dirty for the current root signature.
	// Note: Descriptor table state is undefined at the beginning of a command list and after descriptor heaps are changed on a command list.
	// This will cause the next call to ApplyState to copy and set these descriptors again.
	PipelineState.Common.SRVCache.DirtyAll();
	PipelineState.Common.UAVCache.DirtyAll();
	PipelineState.Common.CBVCache.DirtyAll(GDescriptorTableCBVSlotMask);	// Only mark descriptor table slots as dirty.
}

void CommandContextStateCache::DirtySamplerDescriptorTables()
{
	// Mark the sampler descriptor tables dirty for the current root signature.
	// Note: Descriptor table state is undefined at the beginning of a command list and after descriptor heaps are changed on a command list.
	// This will cause the next call to ApplyState to copy and set these descriptors again.
	PipelineState.Common.SamplerCache.DirtyAll();
}

bool CommandContextStateCache::AssertResourceStates(CachePipelineType PipelineType)
{
#ifdef NDEBUG
	return true;
#else
	ID3D12CommandList* pCommandList = CmdContext->CommandListHandle.CommandList();
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
			if ((UAVInitialCountArray[i] != -1))
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
	auto& CommandList = CmdContext->CommandListHandle;

	const RootSignature* pRootSignature = nullptr;

	if (PipelineType == CPT_Graphics)
	{
		pRootSignature = GetGraphicsRootSignature();

		// See if we need to set a graphics root signature
		if (PipelineState.Graphics.bNeedSetRootSignature)
		{
			CommandList->SetGraphicsRootSignature(pRootSignature->GetSignature());
			PipelineState.Graphics.bNeedSetRootSignature = false;

			// After setting a root signature, all root parameters are undefined and must be set again.
			PipelineState.Common.SRVCache.DirtyGraphics();
			PipelineState.Common.UAVCache.DirtyGraphics();
			PipelineState.Common.SamplerCache.DirtyGraphics();
			PipelineState.Common.CBVCache.DirtyGraphics();
		}
	}
	else if (PipelineType == CPT_Compute)
	{
		pRootSignature = PipelineState.Compute.CurrentPipelineStateObject->ComputeShader->pRootSignature;

		// See if we need to set a compute root signature
		if (PipelineState.Compute.bNeedSetRootSignature)
		{
			CommandList->SetComputeRootSignature(pRootSignature->GetSignature());
			PipelineState.Compute.bNeedSetRootSignature = false;

			// After setting a root signature, all root parameters are undefined and must be set again.
			PipelineState.Common.SRVCache.DirtyCompute();
			PipelineState.Common.UAVCache.DirtyCompute();
			PipelineState.Common.SamplerCache.DirtyCompute();
			PipelineState.Common.CBVCache.DirtyCompute();
		}
	}

	// Ensure the correct graphics or compute PSO is set.
	InternalSetPipelineState<PipelineType>();

	// Need to cache compute budget, as we need to reset after PSO changes
	if (PipelineType == CPT_Compute && CommandList->GetType() == D3D12_COMMAND_LIST_TYPE_COMPUTE)
	{
		CmdContext->SetAsyncComputeBudgetInternal(PipelineState.Compute.ComputeBudget);
	}

	if (PipelineType == CPT_Graphics)
	{
		// Setup non-heap bindings
		if (bNeedSetVB)
		{
			bNeedSetVB = false;
			//SCOPE_CYCLE_COUNTER(STAT_D3D12ApplyStateSetVertexBufferTime);
			DescriptorCache.SetVertexBuffers(PipelineState.Graphics.VBCache);
		}
		if (bNeedSetSOs)
		{
			bNeedSetSOs = false;
			DescriptorCache.SetStreamOutTargets(PipelineState.Graphics.CurrentStreamOutTargets, PipelineState.Graphics.CurrentNumberOfStreamOutTargets, PipelineState.Graphics.CurrentSOOffsets);
		}
		if (bNeedSetViewports)
		{
			bNeedSetViewports = false;
			CommandList->RSSetViewports(PipelineState.Graphics.CurrentNumberOfViewports, PipelineState.Graphics.CurrentViewport);
		}
		if (bNeedSetScissorRects)
		{
			bNeedSetScissorRects = false;
			CommandList->RSSetScissorRects(PipelineState.Graphics.CurrentNumberOfScissorRects, PipelineState.Graphics.CurrentScissorRects);
		}
		if (bNeedSetPrimitiveTopology)
		{
			bNeedSetPrimitiveTopology = false;
			CommandList->IASetPrimitiveTopology(PipelineState.Graphics.CurrentPrimitiveTopology);
		}
		if (bNeedSetBlendFactor)
		{
			bNeedSetBlendFactor = false;
			CommandList->OMSetBlendFactor(PipelineState.Graphics.CurrentBlendFactor);
		}
		if (bNeedSetStencilRef)
		{
			bNeedSetStencilRef = false;
			CommandList->OMSetStencilRef(PipelineState.Graphics.CurrentReferenceStencil);
		}
		if (bNeedSetRTs)
		{
			bNeedSetRTs = false;
			DescriptorCache.SetRenderTargets(PipelineState.Graphics.RenderTargetArray, PipelineState.Graphics.CurrentNumberOfRenderTargets, PipelineState.Graphics.CurrentDepthStencilTarget);
		}
		if (bNeedSetDepthBounds)
		{
			bNeedSetDepthBounds = false;
			ID3D12GraphicsCommandList1* CommandList1 = nullptr;
			CommandList1->OMSetDepthBounds(PipelineState.Graphics.MinDepth, PipelineState.Graphics.MaxDepth);
		}
	}

	// Note that ray tracing pipeline shares state with compute
	const uint32 StartStage = PipelineType == CPT_Graphics ? 0 : ShaderType::ComputeShader;
	const uint32 EndStage = PipelineType == CPT_Graphics ? ShaderType::ComputeShader : ShaderType::NumStandardType;

	const ShaderType UAVStage = PipelineType == CPT_Graphics ? ShaderType::PixelShader : ShaderType::ComputeShader;

	//
	// Reserve space in descriptor heaps
	// Since this can cause heap rollover (which causes old bindings to become invalid), the reserve must be done atomically
	//

	// Samplers
	ApplySamplers(pRootSignature, StartStage, EndStage);

	// Determine what resource bind slots are dirty for the current shaders and how many descriptor table slots we need.
	// We only set dirty resources that can be used for the upcoming Draw/Dispatch.
	SRVSlotMask CurrentShaderDirtySRVSlots[ShaderType::NumStandardType] = {};
	CBVSlotMask CurrentShaderDirtyCBVSlots[ShaderType::NumStandardType] = {};
	UAVSlotMask CurrentShaderDirtyUAVSlots = 0;
	uint32 NumUAVs = 0;
	uint32 NumSRVs[ShaderType::NumStandardType] = {};

	uint32 NumViews = 0;
	for (uint32 iTries = 0; iTries < 2; ++iTries)
	{
		const UAVSlotMask CurrentShaderUAVRegisterMask = ((UAVSlotMask)1 << PipelineState.Common.CurrentShaderUAVCounts[UAVStage]) - (UAVSlotMask)1;
		CurrentShaderDirtyUAVSlots = CurrentShaderUAVRegisterMask & PipelineState.Common.UAVCache.DirtySlotMask[UAVStage];
		if (CurrentShaderDirtyUAVSlots)
		{
			if (ResourceBindingTier <= D3D12_RESOURCE_BINDING_TIER_2)
			{
				// Tier 1 and 2 HW requires the full number of UAV descriptors defined in the root signature's descriptor table.
				NumUAVs = pRootSignature->MaxUAVCount(UAVStage);
			}
			else
			{
				NumUAVs = PipelineState.Common.CurrentShaderUAVCounts[UAVStage];
			}

			lconstraint(NumUAVs > 0 && NumUAVs <= MAX_UAVS);
			NumViews += NumUAVs;
		}

		for (uint32 Stage = StartStage; Stage < EndStage; ++Stage)
		{
			// Note this code assumes the starting register is index 0.
			const SRVSlotMask CurrentShaderSRVRegisterMask = BitMask<SRVSlotMask>(PipelineState.Common.CurrentShaderSRVCounts[Stage]);
			CurrentShaderDirtySRVSlots[Stage] = CurrentShaderSRVRegisterMask & PipelineState.Common.SRVCache.DirtySlotMask[Stage];
			if (CurrentShaderDirtySRVSlots[Stage])
			{
				if (ResourceBindingTier == D3D12_RESOURCE_BINDING_TIER_1)
				{
					// Tier 1 HW requires the full number of SRV descriptors defined in the root signature's descriptor table.
					NumSRVs[Stage] = pRootSignature->MaxSRVCount(Stage);
				}
				else
				{
					NumSRVs[Stage] = PipelineState.Common.CurrentShaderSRVCounts[Stage];
				}

				lconstraint(NumSRVs[Stage] > 0 && NumSRVs[Stage] <= MAX_SRVS);
				NumViews += NumSRVs[Stage];
			}

			const CBVSlotMask CurrentShaderCBVRegisterMask = BitMask<CBVSlotMask>(PipelineState.Common.CurrentShaderCBCounts[Stage]);
			CurrentShaderDirtyCBVSlots[Stage] = CurrentShaderCBVRegisterMask & PipelineState.Common.CBVCache.DirtySlotMask[Stage];

			// Note: CBVs don't currently use descriptor tables but we still need to know what resource point slots are dirty.
		}

		// See if the descriptor slots will fit
		if (!DescriptorCache.GetCurrentViewHeap()->CanReserveSlots(NumViews))
		{
			const bool bDescriptorHeapsChanged = DescriptorCache.GetCurrentViewHeap()->RollOver();
			if (bDescriptorHeapsChanged)
			{
				// If descriptor heaps changed, then all our tables are dirty again and we need to recalculate the number of slots we need.
				NumViews = 0;
				continue;
			}
		}

		// We can reserve slots in the descriptor heap, no need to loop again.
		break;
	}

	uint32 ViewHeapSlot = DescriptorCache.GetCurrentViewHeap()->ReserveSlots(NumViews);

	// Unordered access views
	if (CurrentShaderDirtyUAVSlots)
	{
		DescriptorCache.SetUAVs<UAVStage>(pRootSignature, PipelineState.Common.UAVCache, CurrentShaderDirtyUAVSlots, NumUAVs, ViewHeapSlot);
	}

	// Shader resource views
	{
		auto& SRVCache = PipelineState.Common.SRVCache;

#define CONDITIONAL_SET_SRVS(Shader) \
		if (CurrentShaderDirtySRVSlots[ShaderType::##Shader]) \
		{ \
			DescriptorCache.SetSRVs<ShaderType::##Shader>(pRootSignature, SRVCache, CurrentShaderDirtySRVSlots[ShaderType::##Shader], NumSRVs[ShaderType::##Shader], ViewHeapSlot); \
		}

		if (PipelineType == CPT_Graphics)
		{
			CONDITIONAL_SET_SRVS(VertexShader);
			CONDITIONAL_SET_SRVS(HullShader);
			CONDITIONAL_SET_SRVS(DomainShader);
			CONDITIONAL_SET_SRVS(GeometryShader);
			CONDITIONAL_SET_SRVS(PixelShader);
		}
		else
		{
			// Note that ray tracing pipeline shares state with compute
			CONDITIONAL_SET_SRVS(ComputeShader);
		}
#undef CONDITIONAL_SET_SRVS
	}

	// Constant buffers
	{
		auto& CBVCache = PipelineState.Common.CBVCache;

#define CONDITIONAL_SET_CBVS(Shader) \
		if (CurrentShaderDirtyCBVSlots[ShaderType::##Shader]) \
		{ \
			DescriptorCache.SetConstantBuffers<ShaderType::##Shader>(pRootSignature, CBVCache, CurrentShaderDirtyCBVSlots[ShaderType::##Shader]); \
		}

		if (PipelineType == CPT_Graphics)
		{
			CONDITIONAL_SET_CBVS(VertexShader);
			CONDITIONAL_SET_CBVS(HullShader);
			CONDITIONAL_SET_CBVS(DomainShader);
			CONDITIONAL_SET_CBVS(GeometryShader);
			CONDITIONAL_SET_CBVS(PixelShader);
		}
		else
		{
			// Note that ray tracing pipeline shares state with compute
			CONDITIONAL_SET_CBVS(ComputeShader);
		}
#undef CONDITIONAL_SET_CBVS
	}

	// Flush any needed resource barriers
	CommandList.FlushResourceBarriers();
}



template void CommandContextStateCache::SetShaderResourceView<ShaderType::VertexShader>(ShaderResourceView* SRV, uint32 ResourceIndex);
template void CommandContextStateCache::SetShaderResourceView<ShaderType::PixelShader>(ShaderResourceView* SRV, uint32 ResourceIndex);
template void CommandContextStateCache::SetShaderResourceView<ShaderType::ComputeShader>(ShaderResourceView* SRV, uint32 ResourceIndex);

template void CommandContextStateCache::ApplyState<CPT_Graphics>();
template void CommandContextStateCache::ApplyState<CPT_Compute>();

template void CommandContextStateCache::SetUAVs<ShaderType::PixelShader>(uint32 UAVStartSlot, uint32 NumSimultaneousUAVs, UnorderedAccessView* const* UAVArray, uint32* UAVInitialCountArray);
template void CommandContextStateCache::SetUAVs<ShaderType::ComputeShader>(uint32 UAVStartSlot, uint32 NumSimultaneousUAVs, UnorderedAccessView* const* UAVArray, uint32* UAVInitialCountArray);

template void CommandContextStateCache::ClearUAVs<ShaderType::PixelShader>();
template void CommandContextStateCache::ClearUAVs<ShaderType::ComputeShader>();
