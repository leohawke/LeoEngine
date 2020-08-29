#include "CommandContext.h"
#include "Texture.h"
#include "NodeDevice.h"
#include "Adapter.h"
#include "CommandListManager.h"

using namespace platform_ex::Windows::D3D12;

constexpr auto MaxSimultaneousRenderTargets = platform::Render::MaxSimultaneousRenderTargets;

CommandContextBase::CommandContextBase(D3D12Adapter* InParent, GPUMaskType InGPUMask, bool InIsDefaultContext, bool InIsAsyncComputeContext)
	:AdapterChild(InParent)
	,bIsDefaultContext(InIsDefaultContext)
	,bIsAsyncComputeContext(InIsAsyncComputeContext)
{
}

CommandContext::CommandContext(NodeDevice* InParent, SubAllocatedOnlineHeap::SubAllocationDesc& SubHeapDesc, bool InIsDefaultContext, bool InIsAsyncComputeContext)
	:
	CommandContextBase(InParent->GetParentAdapter(), InParent->GetGPUMask(),InIsDefaultContext, InIsAsyncComputeContext),
	DeviceChild(InParent),
	VSConstantBuffer(InParent, ConstantsAllocator),
	HSConstantBuffer(InParent, ConstantsAllocator),
	DSConstantBuffer(InParent, ConstantsAllocator),
	PSConstantBuffer(InParent, ConstantsAllocator),
	GSConstantBuffer(InParent, ConstantsAllocator),
	CSConstantBuffer(InParent, ConstantsAllocator),
	StateCache(0)
{
	StateCache.Init(InParent, this, nullptr, SubHeapDesc);
}

void CommandContext::BeginRenderPass(const platform::Render::RenderPassInfo& Info, const char* Name)
{
	platform::Render::RenderTargetsInfo RTInfo(Info);

	SetRenderTargetsAndClear(RTInfo);
}

void CommandContext::SetRenderTargetsAndClear(const platform::Render::RenderTargetsInfo& RenderTargetsInfo)
{
	SetRenderTargets(RenderTargetsInfo.NumColorRenderTargets,
		RenderTargetsInfo.ColorRenderTarget,
		&RenderTargetsInfo.DepthStencilRenderTarget);

	leo::math::float4 ClearColors[MaxSimultaneousRenderTargets];

	if (RenderTargetsInfo.bClearColor)
	{
		for (int Index = 0; Index < RenderTargetsInfo.NumColorRenderTargets; ++Index)
		{
			//TODO clear value binding

			ClearColors[Index] = leo::math::float4();
		}
	}

	float DepthClear = 1.0;
	uint32 StencilClear = 0;

	if (RenderTargetsInfo.bClearDepth || RenderTargetsInfo.bClearStencil)
	{
		//TODO clear value binding
	}

	ClearMRT(RenderTargetsInfo.bClearColor, RenderTargetsInfo.NumColorRenderTargets, ClearColors, RenderTargetsInfo.bClearDepth, DepthClear, RenderTargetsInfo.bClearStencil, StencilClear);
}

void CommandContext::SetViewport(uint32 MinX, uint32 MinY, float MinZ, uint32 MaxX, uint32 MaxY, float MaxZ)
{
	// These are the maximum viewport extents for D3D12. Exceeding them leads to badness.
	lconstraint(MinX <= (uint32)D3D12_VIEWPORT_BOUNDS_MAX);
	lconstraint(MinY <= (uint32)D3D12_VIEWPORT_BOUNDS_MAX);
	lconstraint(MaxX <= (uint32)D3D12_VIEWPORT_BOUNDS_MAX);
	lconstraint(MaxY <= (uint32)D3D12_VIEWPORT_BOUNDS_MAX);

	D3D12_VIEWPORT Viewport = { (float)MinX, (float)MinY, (float)(MaxX - MinX), (float)(MaxY - MinY), MinZ, MaxZ };

	if (Viewport.Width > 0 && Viewport.Height > 0)
	{
		StateCache.SetViewport(Viewport);
		SetScissorRect(true, MinX, MinY, MaxX, MaxY);
	}
}

void CommandContext::SetScissorRect(bool bEnable, uint32 MinX, uint32 MinY, uint32 MaxX, uint32 MaxY)
{
	if (bEnable)
	{
		const CD3DX12_RECT ScissorRect(MinX, MinY, MaxX, MaxY);
		StateCache.SetScissorRect(ScissorRect);
	}
	else
	{
		const D3D12_VIEWPORT& Viewport = StateCache.GetViewport();
		const CD3DX12_RECT ScissorRect((LONG)Viewport.TopLeftX, (LONG)Viewport.TopLeftY, (LONG)Viewport.TopLeftX + (LONG)Viewport.Width, (LONG)Viewport.TopLeftY + (LONG)Viewport.Height);
		StateCache.SetScissorRect(ScissorRect);
	}
}

void CommandContext::SetVertexBuffer(uint32 slot, platform::Render::GraphicsBuffer* IVertexBuffer)
{
	auto VertexBuffer = static_cast<GraphicsBuffer*>(IVertexBuffer);

	StateCache.SetStreamSource(VertexBuffer, slot, 0);
}

void CommandContext::SetGraphicsPipelineState(platform::Render::GraphicsPipelineState* pso)
{
	auto PipelineState = static_cast<GraphicsPipelineState*>(pso);

	const bool bWasUsingTessellation = bUsingTessellation;
	bUsingTessellation = PipelineState->GetHullShader() && PipelineState->GetDomainShader();

	// Ensure the command buffers are reset to reduce the amount of data that needs to be versioned.
	VSConstantBuffer.Reset();
	PSConstantBuffer.Reset();
	HSConstantBuffer.Reset();
	DSConstantBuffer.Reset();
	GSConstantBuffer.Reset();
	// Should this be here or in RHISetComputeShader? Might need a new bDiscardSharedConstants for CS.
	CSConstantBuffer.Reset();

	//really should only discard the constants if the shader state has actually changed.
	bDiscardSharedConstants = true;

	if (!false)
	{
		StateCache.SetDepthBounds(0.0f, 1.0f);
	}

	StateCache.SetGraphicsPipelineState(PipelineState, bUsingTessellation != bWasUsingTessellation);
	StateCache.SetStencilRef(0);
}

void CommandContext::SetShaderSampler(platform::Render::VertexHWShader* Shader, uint32 SamplerIndex, const platform::Render::TextureSampleDesc& Desc)
{
	auto pSampler = GetParentDevice()->CreateSampler(Convert(Desc));

	StateCache.SetSamplerState<ShaderType::VertexShader>(pSampler.get(), SamplerIndex);
}

void CommandContext::SetShaderSampler(platform::Render::PixelHWShader* Shader, uint32 SamplerIndex, const platform::Render::TextureSampleDesc& Desc)
{
	auto pSampler = GetParentDevice()->CreateSampler(Convert(Desc));

	StateCache.SetSamplerState<ShaderType::PixelShader>(pSampler.get(), SamplerIndex);
}

void CommandContext::SetShaderTexture(platform::Render::VertexHWShader* Shader, uint32 TextureIndex, platform::Render::Texture* ITexture)
{
	auto SRV = dynamic_cast_texture(ITexture)->RetriveShaderResourceView();

	StateCache.SetShaderResourceView<ShaderType::VertexShader>(SRV, TextureIndex);
}

void CommandContext::SetShaderTexture(platform::Render::PixelHWShader* Shader, uint32 TextureIndex, platform::Render::Texture* ITexture)
{
	auto SRV = dynamic_cast_texture(ITexture)->RetriveShaderResourceView();

	StateCache.SetShaderResourceView<ShaderType::PixelShader>(SRV, TextureIndex);
}

void CommandContext::SetShaderResourceView(platform::Render::VertexHWShader* Shader, uint32 TextureIndex, platform::Render::ShaderResourceView* SRV)
{
	StateCache.SetShaderResourceView<ShaderType::VertexShader>(static_cast<ShaderResourceView*>(SRV), TextureIndex);
}

void CommandContext::SetShaderResourceView(platform::Render::PixelHWShader* Shader, uint32 TextureIndex, platform::Render::ShaderResourceView* SRV)
{
	StateCache.SetShaderResourceView<ShaderType::PixelShader>(static_cast<ShaderResourceView*>(SRV), TextureIndex);
}

void CommandContext::SetShaderConstantBuffer(platform::Render::VertexHWShader* Shader, uint32 BaseIndex, platform::Render::GraphicsBuffer* IBuffer)
{
	auto Buffer = static_cast<GraphicsBuffer*>(IBuffer);

	StateCache.SetConstantsBuffer<ShaderType::VertexShader>(BaseIndex, Buffer);

	BoundConstantBuffers[ShaderType::VertexShader][BaseIndex] = Buffer;
	DirtyConstantBuffers[ShaderType::VertexShader] |= (1 << BaseIndex);
}

void CommandContext::SetShaderConstantBuffer(platform::Render::PixelHWShader* Shader, uint32 BaseIndex, platform::Render::GraphicsBuffer* IBuffer)
{
	auto Buffer = static_cast<GraphicsBuffer*>(IBuffer);

	StateCache.SetConstantsBuffer<ShaderType::PixelShader>(BaseIndex, Buffer);

	BoundConstantBuffers[ShaderType::PixelShader][BaseIndex] = Buffer;
	DirtyConstantBuffers[ShaderType::PixelShader] |= (1 << BaseIndex);
}

void CommandContext::SetShaderParameter(platform::Render::VertexHWShader* Shader, uint32 BufferIndex, uint32 BaseIndex, uint32 NumBytes, const void* NewValue)
{
	lconstraint(BufferIndex == 0);
	VSConstantBuffer.UpdateConstant(reinterpret_cast<const uint8*>(NewValue), BaseIndex, NumBytes);
}

void CommandContext::SetShaderParameter(platform::Render::PixelHWShader* Shader, uint32 BufferIndex, uint32 BaseIndex, uint32 NumBytes, const void* NewValue)
{
	lconstraint(BufferIndex == 0);
	PSConstantBuffer.UpdateConstant(reinterpret_cast<const uint8*>(NewValue), BaseIndex, NumBytes);
}

void CommandContext::CommitGraphicsResourceTables()
{
	//don't support UE4 ShaderResourceTable
}

void CommandContext::CommitNonComputeShaderConstants()
{
	const auto* const  GraphicPSO = StateCache.GetGraphicsPipelineState();

	lconstraint(GraphicPSO);

	// Only set the constant buffer if this shader needs the global constant buffer bound
	// Otherwise we will overwrite a different constant buffer
	if (GraphicPSO->bShaderNeedsGlobalConstantBuffer[ShaderType::VertexShader])
	{
		StateCache.SetConstantBuffer<ShaderType::VertexShader>(VSConstantBuffer, bDiscardSharedConstants);
	}

	// Skip HS/DS CB updates in cases where tessellation isn't being used
	// Note that this is *potentially* unsafe because bDiscardSharedConstants is cleared at the
	// end of the function, however we're OK for now because bDiscardSharedConstants
	// is always reset whenever bUsingTessellation changes in SetBoundShaderState()
	if (bUsingTessellation)
	{
		if (GraphicPSO->bShaderNeedsGlobalConstantBuffer[ShaderType::HullShader])
		{
			StateCache.SetConstantBuffer<ShaderType::HullShader>(HSConstantBuffer, bDiscardSharedConstants);
		}

		if (GraphicPSO->bShaderNeedsGlobalConstantBuffer[ShaderType::DomainShader])
		{
			StateCache.SetConstantBuffer<ShaderType::DomainShader>(DSConstantBuffer, bDiscardSharedConstants);
		}
	}

	if (GraphicPSO->bShaderNeedsGlobalConstantBuffer[ShaderType::GeometryShader])
	{
		StateCache.SetConstantBuffer<ShaderType::GeometryShader>(GSConstantBuffer, bDiscardSharedConstants);
	}

	if (GraphicPSO->bShaderNeedsGlobalConstantBuffer[ShaderType::PixelShader])
	{
		StateCache.SetConstantBuffer<ShaderType::PixelShader>(PSConstantBuffer, bDiscardSharedConstants);
	}

	bDiscardSharedConstants = false;
}

void CommandContext::OpenCommandList()
{
	ConditionalObtainCommandAllocator();

	CommandListHandle = GetCommandListManager().ObtainCommandList(*CommandAllocator);
	CommandListHandle.SetCurrentOwningContext(this);

	// Notify the descriptor cache about the new command list
	// This will set the descriptor cache's current heaps on the new command list.
	StateCache.GetDescriptorCache()->NotifyCurrentCommandList(CommandListHandle);

	// Go through the state and find bits that differ from command list defaults.
	// Mark state as dirty so next time ApplyState is called, it will set all state on this new command list
	StateCache.DirtyStateForNewCommandList();

	numDraws = 0;
	numBarriers = 0;
}

void CommandContext::CloseCommandList()
{
	CommandListHandle.Close();
}

CommandListHandle CommandContext::FlushCommands(bool WaitForCompletion)
{
	auto Device = GetParentDevice();
	const bool bHasPendingWork = Device->PendingCommandLists.size() > 0;
	const bool bHasDoneWork = HasDoneWork() || bHasPendingWork;
	const bool bOpenNewCmdList = WaitForCompletion || bHasDoneWork;

	// Only submit a command list if it does meaningful work or the flush is expected to wait for completion.
	if (bOpenNewCmdList)
	{
		// Close the current command list
		CloseCommandList();

		if (bHasPendingWork)
		{
			// Submit all pending command lists and the current command list
			Device->PendingCommandLists.emplace_back(CommandListHandle);
			GetCommandListManager().ExecuteCommandLists(Device->PendingCommandLists, WaitForCompletion);
			Device->PendingCommandLists.clear();
		}
		else
		{
			// Just submit the current command list
			CommandListHandle.Execute(WaitForCompletion);
		}

		// Get a new command list to replace the one we submitted for execution. 
		// Restore the state from the previous command list.
		OpenCommandList();
	}

	return CommandListHandle;
}

void CommandContext::BeginFrame()
{
	auto GPUIndex = 0;

	auto Device = ParentAdapter->GetNodeDevice(GPUIndex);

	auto& SamplerHeap = Device->GetGlobalSamplerHeap();

	if (SamplerHeap.DescriptorTablesDirty())
	{
		SamplerHeap.GetUniqueDescriptorTables().rehash(0);
	}

	const uint32 NumContexts = Device->GetNumContexts();
	for (uint32 i = 0; i < NumContexts; ++i)
	{
		Device->GetCommandContext(i).StateCache.GetDescriptorCache()->BeginFrame();
	}

	Device->GetGlobalSamplerHeap().ToggleDescriptorTablesDirtyFlag(false);

	//TODO GPUProfiler
}

void CommandContext::EndFrame()
{
	//CleraPSOCache
	auto GPUIndex = 0;

	auto Device = ParentAdapter->GetNodeDevice(GPUIndex);
	auto& DefaultContext = Device->GetDefaultCommandContext();
	DefaultContext.CommandListHandle.FlushResourceBarriers();

	DefaultContext.ReleaseCommandAllocator();
	DefaultContext.ClearState();
	DefaultContext.FlushCommands();

	const uint32 NumContexts = Device->GetNumContexts();
	for (uint32 i = 0; i < NumContexts; ++i)
	{
		auto& CommandContext = Device->GetCommandContext(i);
		CommandContext.StateCache.GetDescriptorCache()->EndFrame();

		if (!CommandContext.IsDefaultContext())
		{
			CommandContext.ReleaseCommandAllocator();
		}
	}
}

void CommandContext::ClearState()
{
	StateCache.ClearState();

	bDiscardSharedConstants = false;

	std::memset(BoundConstantBuffers, 0, sizeof(BoundConstantBuffers));
	std::memset(DirtyConstantBuffers, 0, sizeof(DirtyConstantBuffers));

	if (!bIsAsyncComputeContext)
	{

	}
}

CommandListManager& CommandContext::GetCommandListManager()
{
	return GetParentDevice()->GetCommandListManager();
}

void CommandContext::ConditionalObtainCommandAllocator()
{
	if (CommandAllocator == nullptr)
	{
		// Obtain a command allocator if the context doesn't already have one.
		// This will check necessary fence values to ensure the returned command allocator isn't being used by the GPU, then reset it.
		CommandAllocator = new D3D12::CommandAllocator(GetParentDevice()->GetDevice(), D3D12_COMMAND_LIST_TYPE_DIRECT);
	}
}

void CommandContext::ReleaseCommandAllocator()
{
	if (CommandAllocator != nullptr)
	{
		// Release the command allocator so it can be reused.
	}
}

static uint32 GetIndexCount(platform::Render::PrimtivteType type,uint32 NumPrimitives)
{
	auto PrimitiveTypeFactor = platform::Render::GetPrimitiveTypeFactor(type);

	auto PrimitiveTypeOffset = type == platform::Render::PrimtivteType::TriangleStrip ? 2 : 0;

	return PrimitiveTypeFactor * NumPrimitives + PrimitiveTypeOffset;
}

void CommandContext::DrawIndexedPrimitive(platform::Render::GraphicsBuffer* IIndexBuffer, int32 BaseVertexIndex, uint32 FirstInstance, uint32 NumVertices, uint32 StartIndex, uint32 NumPrimitives, uint32 NumInstances)
{
	// called should make sure the input is valid, this avoid hidden bugs
	lconstraint(NumPrimitives > 0);

	NumInstances = std::max<uint32>(1, NumInstances);
	numDraws++;

	CommitGraphicsResourceTables();
	CommitNonComputeShaderConstants();

	uint32 IndexCount = GetIndexCount(StateCache.GetPrimtivteType(),NumPrimitives);
	auto IndexBuffer = static_cast<GraphicsBuffer*>(IIndexBuffer);

	const DXGI_FORMAT Format = Convert(IndexBuffer->GetFormat());
	StateCache.SetIndexBuffer(IndexBuffer, Format, 0);
	StateCache.ApplyState<CPT_Graphics>();

	CommandListHandle->DrawIndexedInstanced(IndexCount, NumInstances, StartIndex, BaseVertexIndex, FirstInstance);
}

void CommandContext::DrawPrimitive(uint32 BaseVertexIndex, uint32 FirstInstance, uint32 NumPrimitives, uint32 NumInstances)
{
	NumInstances = std::max<uint32>(1, NumInstances);
	numDraws++;

	CommitGraphicsResourceTables();
	CommitNonComputeShaderConstants();

	uint32 VertexCount = GetIndexCount(StateCache.GetPrimtivteType(), NumPrimitives);

	StateCache.ApplyState<CPT_Graphics>();
	CommandListHandle->DrawInstanced(VertexCount, NumInstances, BaseVertexIndex, FirstInstance);
}

struct FRTVDesc
{
	uint32 Width;
	uint32 Height;
	DXGI_SAMPLE_DESC SampleDesc;
};

// Return an FRTVDesc structure whose
// Width and height dimensions are adjusted for the RTV's miplevel.
FRTVDesc GetRenderTargetViewDesc(RenderTargetView* RenderTargetView)
{
	const D3D12_RENDER_TARGET_VIEW_DESC& TargetDesc = RenderTargetView->GetDesc();

	auto BaseResource = RenderTargetView->GetResource();
	uint32 MipIndex = 0;
	FRTVDesc ret;
	memset(&ret, 0, sizeof(ret));

	switch (TargetDesc.ViewDimension)
	{
	case D3D12_RTV_DIMENSION_TEXTURE2D:
	case D3D12_RTV_DIMENSION_TEXTURE2DMS:
	case D3D12_RTV_DIMENSION_TEXTURE2DARRAY:
	case D3D12_RTV_DIMENSION_TEXTURE2DMSARRAY:
	{
		D3D12_RESOURCE_DESC const& Desc = BaseResource->GetDesc();
		ret.Width = (uint32)Desc.Width;
		ret.Height = Desc.Height;
		ret.SampleDesc = Desc.SampleDesc;
		if (TargetDesc.ViewDimension == D3D12_RTV_DIMENSION_TEXTURE2D || TargetDesc.ViewDimension == D3D12_RTV_DIMENSION_TEXTURE2DARRAY)
		{
			// All the non-multisampled texture types have their mip-slice in the same position.
			MipIndex = TargetDesc.Texture2D.MipSlice;
		}
		break;
	}
	case D3D12_RTV_DIMENSION_TEXTURE3D:
	{
		D3D12_RESOURCE_DESC const& Desc = BaseResource->GetDesc();
		ret.Width = (uint32)Desc.Width;
		ret.Height = Desc.Height;
		ret.SampleDesc.Count = 1;
		ret.SampleDesc.Quality = 0;
		MipIndex = TargetDesc.Texture3D.MipSlice;
		break;
	}
	default:
	{
		// not expecting 1D targets.
		lconstraint(false);
	}
	}
	ret.Width >>= MipIndex;
	ret.Height >>= MipIndex;
	return ret;
}


void CommandContext::SetRenderTargets(uint32 NewNumSimultaneousRenderTargets, const platform::Render::RenderTarget* NewRenderTargets, const platform::Render::DepthRenderTarget* INewDepthStencilTarget)
{
	auto NewDepthStencilTarget = INewDepthStencilTarget ? dynamic_cast<Texture*>(INewDepthStencilTarget->Texture) : nullptr;

	bool bTargetChanged = false;

	DepthStencilView* DepthStencilView = nullptr;
	if (NewDepthStencilTarget)
	{
		DepthStencilView = NewDepthStencilTarget->GetDepthStencilView({});
	}

	// Check if the depth stencil target is different from the old state.
	if (CurrentDepthStencilTarget != DepthStencilView)
	{
		CurrentDepthTexture = NewDepthStencilTarget;
		CurrentDepthStencilTarget = DepthStencilView;
		bTargetChanged = true;
	}

	// Gather the render target views for the new render targets.
	RenderTargetView* NewRenderTargetViews[MaxSimultaneousRenderTargets];
	for (uint32 RenderTargetIndex = 0; RenderTargetIndex < MaxSimultaneousRenderTargets; ++RenderTargetIndex)
	{
		RenderTargetView* RenderTargetView = nullptr;

		if (RenderTargetIndex < NewNumSimultaneousRenderTargets && NewRenderTargets[RenderTargetIndex].Texture != nullptr)
		{
			int32 RTMipIndex = NewRenderTargets[RenderTargetIndex].MipIndex;
			int32 RTSliceIndex = NewRenderTargets[RenderTargetIndex].ArraySlice;

			auto NewRenderTarget = dynamic_cast<Texture*>(NewRenderTargets[RenderTargetIndex].Texture);

			RenderTargetView = NewRenderTarget->GetRenderTargetView(RTMipIndex, RTSliceIndex);

			LAssert(RenderTargetView, "Texture being set as render target has no RTV");
		}

		NewRenderTargetViews[RenderTargetIndex] = RenderTargetView;
		// Check if the render target is different from the old state.
		if (CurrentRenderTargets[RenderTargetIndex] != RenderTargetView)
		{
			CurrentRenderTargets[RenderTargetIndex] = RenderTargetView;
			bTargetChanged = true;
		}
	}

	if (NumSimultaneousRenderTargets != NewNumSimultaneousRenderTargets)
	{
		NumSimultaneousRenderTargets = NewNumSimultaneousRenderTargets;
		bTargetChanged = true;
	}

	if (bTargetChanged)
	{
		StateCache.SetRenderTargets(NewNumSimultaneousRenderTargets, CurrentRenderTargets, CurrentDepthStencilTarget);
		StateCache.ClearUAVs<ShaderType::PixelShader>();
	}

	// Set the viewport to the full size of render target 0.
	if (NewRenderTargetViews[0])
	{
		// check target 0 is valid
		lconstraint(0 < NewNumSimultaneousRenderTargets && NewRenderTargets[0].Texture != nullptr);
		FRTVDesc RTTDesc = GetRenderTargetViewDesc(NewRenderTargetViews[0]);
		SetViewport(0, 0, 0.0f, RTTDesc.Width, RTTDesc.Height, 1.0f);
	}
	else if (DepthStencilView)
	{
		auto DepthTargetTexture = DepthStencilView->GetResource();
		D3D12_RESOURCE_DESC const& DTTDesc = DepthTargetTexture->GetDesc();
		SetViewport(0, 0, 0.0f, static_cast<uint32>(DTTDesc.Width), DTTDesc.Height, 1.0f);
	}
}

void CommandContext::ClearMRT(bool bClearColor, int32 NumClearColors, const leo::math::float4* ColorArray, bool bClearDepth, float Depth, bool bClearStencil, uint32 Stencil)
{
	const D3D12_VIEWPORT& Viewport = StateCache.GetViewport();
	const D3D12_RECT& ScissorRect = StateCache.GetScissorRect();

	if (ScissorRect.left >= ScissorRect.right || ScissorRect.top >= ScissorRect.bottom)
	{
		return;
	}

	RenderTargetView* RenderTargetViews[MaxSimultaneousRenderTargets];
	DepthStencilView* DSView = nullptr;
	uint32 NumSimultaneousRTs = 0;
	StateCache.GetRenderTargets(RenderTargetViews, &NumSimultaneousRTs, &DSView);

	const LONG Width = static_cast<LONG>(Viewport.Width);
	const LONG Height = static_cast<LONG>(Viewport.Height);

	bool bClearCoversEntireSurface = false;
	if (ScissorRect.left <= 0 && ScissorRect.top <= 0 &&
		ScissorRect.right >= Width && ScissorRect.bottom >= Height)
	{
		bClearCoversEntireSurface = true;
	}

	const bool bSupportsFastClear = true;
	uint32 ClearRectCount = 0;
	D3D12_RECT* pClearRects = nullptr;
	D3D12_RECT ClearRects[4];

	// Only pass a rect down to the driver if we specifically want to clear a sub-rect
	if (!bSupportsFastClear || !bClearCoversEntireSurface)
	{
		{
			ClearRects[ClearRectCount] = ScissorRect;
			ClearRectCount++;
		}

		pClearRects = ClearRects;
	}

	const bool ClearRTV = bClearColor && NumSimultaneousRTs > 0;
	const bool ClearDSV = (bClearDepth || bClearStencil) && DSView;

	if (ClearRTV)
	{
		for (uint32 TargetIndex = 0; TargetIndex < NumSimultaneousRTs; TargetIndex++)
		{
			auto View = RenderTargetViews[TargetIndex];

			if (View != nullptr)
			{
				TransitionResource(CommandListHandle, View, D3D12_RESOURCE_STATE_RENDER_TARGET);
			}
		}

	}

	uint32 ClearFlags = 0;

	if (ClearDSV)
	{
		if (bClearDepth && DSView->HasDepth())
		{
			ClearFlags |= D3D12_CLEAR_FLAG_DEPTH;
		}

		if (bClearStencil && DSView->HasStencil())
		{
			ClearFlags |= D3D12_CLEAR_FLAG_STENCIL;
		}

		if (bClearDepth && (!DSView->HasStencil() || bClearStencil))
		{
			// Transition the entire view (Both depth and stencil planes if applicable)
			// Some DSVs don't have stencil bits.
			TransitionResource(CommandListHandle, DSView, D3D12_RESOURCE_STATE_DEPTH_WRITE);
		}
		else
		{
			if (bClearDepth)
			{
				// Transition just the depth plane
				TransitionResource(CommandListHandle, DSView->GetResourceLocation(), D3D12_RESOURCE_STATE_DEPTH_WRITE, DSView->GetDepthOnlyViewSubresourceSubset());
			}
			else
			{
				// Transition just the stencil plane
				TransitionResource(CommandListHandle, DSView->GetResourceLocation(), D3D12_RESOURCE_STATE_DEPTH_WRITE, DSView->GetStencilOnlyViewSubresourceSubset());
			}
		}
	}

	if (ClearRTV || ClearDSV)
	{
		CommandListHandle.FlushResourceBarriers();

		if (ClearRTV)
		{
			for (uint32 TargetIndex = 0; TargetIndex < NumSimultaneousRTs; ++TargetIndex)
			{
				auto RTView = RenderTargetViews[TargetIndex];

				if (RTView != nullptr)
				{
					CommandListHandle->ClearRenderTargetView(RTView->GetView(), (float*)&ColorArray[TargetIndex], ClearRectCount, pClearRects);
				}
			}
		}

		if (ClearDSV)
		{
			CommandListHandle->ClearDepthStencilView(DSView->GetView(), (D3D12_CLEAR_FLAGS)ClearFlags, Depth, Stencil, ClearRectCount, pClearRects);
		}
	}
}

void CommandContext::SetComputeShader(platform::Render::ComputeHWShader* ComputeShader)
{
	auto ComputePipelineState = GetParentAdapter()->CreateComputePipelineState(ComputeShader);

	SetComputePipelineState(ComputePipelineState);
}

void CommandContext::DispatchComputeShader(uint32 ThreadGroupCountX, uint32 ThreadGroupCountY, uint32 ThreadGroupCountZ)
{
	ComputeHWShader* ComputeShader = nullptr;
	StateCache.GetComputeShader(&ComputeShader);

	if (ComputeShader->bGlobalUniformBufferUsed)
	{
		CommitComputeShaderConstants();
	}

	CommitComputeResourceTables(ComputeShader);
	StateCache.ApplyState<CPT_Compute>();

	CommandListHandle->Dispatch(ThreadGroupCountX, ThreadGroupCountY, ThreadGroupCountZ);
}

void CommandContext::SetShaderTexture(platform::Render::ComputeHWShader* Shader, uint32 TextureIndex, platform::Render::Texture* ITexture)
{
	auto SRV = dynamic_cast_texture(ITexture)->RetriveShaderResourceView();

	StateCache.SetShaderResourceView<ShaderType::ComputeShader>(SRV, TextureIndex);
}

void CommandContext::SetShaderSampler(platform::Render::ComputeHWShader* Shader, uint32 SamplerIndex, const platform::Render::TextureSampleDesc& Desc)
{
	auto pSampler = GetParentDevice()->CreateSampler(Convert(Desc));

	StateCache.SetSamplerState<ShaderType::ComputeShader>(pSampler.get(), SamplerIndex);
}

void CommandContext::SetUAVParameter(platform::Render::ComputeHWShader* Shader, uint32 UAVIndex, platform::Render::UnorderedAccessView* IUAV)
{
	auto UAV = static_cast<UnorderedAccessView*>(IUAV);

	uint32 InitialCount = -1;

	StateCache.SetUAVs<ShaderType::ComputeShader>(UAVIndex, 1, &UAV, &InitialCount);
}

void CommandContext::SetUAVParameter(platform::Render::ComputeHWShader* Shader, uint32 UAVIndex, platform::Render::UnorderedAccessView* IUAV, uint32 InitialCount)
{
	auto UAV = static_cast<UnorderedAccessView*>(IUAV);

	//Clear ShaderResource

	StateCache.SetUAVs<ShaderType::ComputeShader>(UAVIndex, 1, &UAV, &InitialCount);
}

void CommandContext::SetShaderParameter(platform::Render::ComputeHWShader* Shader, uint32 BufferIndex, uint32 BaseIndex, uint32 NumBytes, const void* NewValue)
{
	lconstraint(BufferIndex == 0);
	CSConstantBuffer.UpdateConstant(reinterpret_cast<const uint8*>(NewValue), BaseIndex, NumBytes);
}

void CommandContext::SetComputePipelineState(ComputePipelineState* ComputeState)
{
	StateCache.TransitionComputeState(CPT_Compute);

	StateCache.SetComputeShader(ComputeState->ComputeShader);
	StateCache.SetComputePipelineState(ComputeState);
}

void CommandContext::CommitComputeShaderConstants()
{
	StateCache.SetConstantBuffer<ShaderType::ComputeShader>(CSConstantBuffer, bDiscardSharedConstants);
}


void CommandContext::CommitComputeResourceTables(ComputeHWShader* ComputeShader)
{
	//don't support UE4 ShaderResourceTable
}