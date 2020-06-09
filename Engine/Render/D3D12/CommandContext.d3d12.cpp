#include "CommandContext.h"
#include "Texture.h"

using namespace platform_ex::Windows::D3D12;

constexpr auto MaxSimultaneousRenderTargets = platform::Render::MaxSimultaneousRenderTargets;

void SetRenderTargetsInfo::ConvertFromPassInfo(const platform::Render::RenderPassInfo& Info)
{
	//TODO RenderTargetActions
	bClearColor = true;
	for (uint32 Index = 0; Index < MaxSimultaneousRenderTargets; ++Index)
	{
		if (!Info.ColorRenderTargets[Index])
			break;

		ColorRenderTargets[Index] =static_cast<RenderTargetView*>(Info.ColorRenderTargets[Index]);
		//TODO Subres

		++NumColorRenderTargets;
	}

	//TODO DepthActions
	DepthStencilTarget = static_cast<DepthStencilView*>(Info.DepthStencilTarget);

	bClearDepth = true;
	bClearStencil = true;

	if (Info.NumUAVs > 0)
	{
		//TODO UAVIndex
		for (uint32 Index = 0; Index != Info.NumUAVs; ++Index)
		{
			UAVs[Index] = static_cast<UnorderedAccessView*>(Info.UAVs[Index]);
		}

		NumUAVs = Info.NumUAVs;
	}
}

CommandContext::CommandContext(D3D12Device* InParent, SubAllocatedOnlineHeap::SubAllocationDesc& SubHeapDesc, bool InIsDefaultContext, bool InIsAsyncComputeContext)
	:
	VSConstantBuffer(InParent, ConstantsAllocator),
	HSConstantBuffer(InParent, ConstantsAllocator),
	DSConstantBuffer(InParent, ConstantsAllocator),
	PSConstantBuffer(InParent, ConstantsAllocator),
	GSConstantBuffer(InParent, ConstantsAllocator),
	CSConstantBuffer(InParent, ConstantsAllocator)
{
}

void CommandContext::BeginRenderPass(const platform::Render::RenderPassInfo& Info, const char* Name)
{
	SetRenderTargetsInfo RTInfo;
	RTInfo.ConvertFromPassInfo(Info);

	SetRenderTargetsAndClear(RTInfo);
}

void CommandContext::SetRenderTargetsAndClear(const SetRenderTargetsInfo& RenderTargetsInfo)
{
	SetRenderTargets(RenderTargetsInfo.NumColorRenderTargets,
		RenderTargetsInfo.ColorRenderTargets,
		RenderTargetsInfo.DepthStencilTarget,
		RenderTargetsInfo.NumUAVs,
		RenderTargetsInfo.UAVs);

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

void CommandContext::SetRenderTargets(uint32 NewNumSimultaneousRenderTargets, const RenderTargetView* const* NewRenderTargets, const DepthStencilView* NewDepthStencilTarget, uint32 NewNumUAVs, UnorderedAccessView* const* UAVs)
{
	//TODO Cache Pre
	StateCache.SetRenderTargets(NewNumSimultaneousRenderTargets, NewRenderTargets,NewDepthStencilTarget);

	if (NewNumUAVs > 0)
	{
		uint32 UAVInitialCountArray[MAX_UAVS];
		for (uint32 UAVIndex = 0; UAVIndex < NewNumUAVs; ++UAVIndex)
		{
			// Using the value that indicates to keep the current UAV counter
			UAVInitialCountArray[UAVIndex] = -1;
		}

		StateCache.SetUAVs<ShaderType::PixelShader>(NewNumSimultaneousRenderTargets, NewNumUAVs, UAVs, UAVInitialCountArray);
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
		if (ClearRTV)
		{
			for (uint32 TargetIndex = 0; TargetIndex < NumSimultaneousRTs; ++TargetIndex)
			{
				auto RTView = RenderTargetViews[TargetIndex];

				if (RTView)
				{
					RTView->ClearColor(ColorArray[TargetIndex]);
				}
			}

			if (ClearDSV)
			{
				DSView->ClearDepthStencil(Depth, Stencil);
			}
		}
	}
}
