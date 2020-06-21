#include "CommandContext.h"
#include "Texture.h"
#include "NodeDevice.h"
#include "CommandListManager.h"

using namespace platform_ex::Windows::D3D12;

constexpr auto MaxSimultaneousRenderTargets = platform::Render::MaxSimultaneousRenderTargets;

CommandContext::CommandContext(NodeDevice* InParent, SubAllocatedOnlineHeap::SubAllocationDesc& SubHeapDesc, bool InIsDefaultContext, bool InIsAsyncComputeContext)
	:
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
			//TODO fast clear support

			ClearColors[Index] = leo::math::float4();
		}
	}

	float DepthClear = 0.0;
	uint32 StencilClear = 0;

	if (RenderTargetsInfo.bClearDepth || RenderTargetsInfo.bClearStencil)
	{
		//TODO fast clear support
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
	StateCache.SetSamplerState<ShaderType::VertexShader>(Desc, SamplerIndex);
}

void CommandContext::SetShaderSampler(platform::Render::PixelHWShader* Shader, uint32 SamplerIndex, const platform::Render::TextureSampleDesc& Desc)
{
	StateCache.SetSamplerState<ShaderType::PixelShader>(Desc, SamplerIndex);
}

void CommandContext::SetShaderTexture(platform::Render::VertexHWShader* Shader, uint32 TextureIndex, platform::Render::Texture* ITexture)
{
	auto SRV = dynamic_cast<Texture*>(ITexture)->RetriveShaderResourceView();

	StateCache.SetShaderResourceView<ShaderType::VertexShader>(SRV, TextureIndex);
}

void CommandContext::SetShaderTexture(platform::Render::PixelHWShader* Shader, uint32 TextureIndex, platform::Render::Texture* ITexture)
{
	auto SRV = dynamic_cast<Texture*>(ITexture)->RetriveShaderResourceView();

	StateCache.SetShaderResourceView<ShaderType::PixelShader>(SRV, TextureIndex);
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
}

void CommandContext::CloseCommandList()
{
	CommandListHandle.Close();
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

void CommandContext::DrawPrimitive(uint32 BaseVertexIndex, uint32 NumPrimitives, uint32 NumInstances)
{
	NumInstances = std::max<uint32>(1, NumInstances);
	numDraws++;

	CommitGraphicsResourceTables();
	CommitNonComputeShaderConstants();

	uint32 VertexCount = GetIndexCount(StateCache.GetPrimtivteType(), NumPrimitives);

	StateCache.ApplyState<CPT_Graphics>();
	CommandListHandle->DrawInstanced(VertexCount, NumInstances, BaseVertexIndex, 0);
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
		SetViewport(0.0f, 0.0f, 0.0f, (float)RTTDesc.Width, (float)RTTDesc.Height, 1.0f);
	}
	else if (DepthStencilView)
	{
		auto DepthTargetTexture = DepthStencilView->GetResource();
		D3D12_RESOURCE_DESC const& DTTDesc = DepthTargetTexture->GetDesc();
		SetViewport(0.0f, 0.0f, 0.0f, (float)DTTDesc.Width, (float)DTTDesc.Height, 1.0f);
	}
}

void CommandContext::ClearMRT(bool bClearColor, int32 NumClearColors, const leo::math::float4* ColorArray, bool bClearDepth, float Depth, bool bClearStencil, uint32 Stencil)
{
	RenderTargetView* RenderTargetViews[MaxSimultaneousRenderTargets];
	DepthStencilView* DSView = nullptr;
	uint32 NumSimultaneousRTs = 0;
	StateCache.GetRenderTargets(RenderTargetViews, &NumSimultaneousRTs, &DSView);

	const bool ClearRTV = bClearColor && NumSimultaneousRTs > 0;
	const bool ClearDSV = (bClearDepth || bClearStencil) && DSView;

	if (ClearRTV || ClearDSV)
	{
		throw leo::unsupported();
		if (ClearRTV)
		{
			for (uint32 TargetIndex = 0; TargetIndex < NumSimultaneousRTs; ++TargetIndex)
			{
				auto RTView = RenderTargetViews[TargetIndex];

				if (RTView)
				{
				}
			}

			if (ClearDSV)
			{
			}
		}
	}
}
