#include "CommandContext.h"

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
		for (uint32 Index = 0; Index < RenderTargetsInfo.NumColorRenderTargets; ++Index)
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

void CommandContext::SetVertexBuffer(uint32 slot, platform::Render::GraphicsBuffer* VertexBuffer)
{
}

void CommandContext::SetGraphicsPipelineState(platform::Render::GraphicsPipelineState* pso)
{
}

void CommandContext::SetShaderSampler(platform::Render::VertexHWShader* Shader, uint32 SamplerIndex, const platform::Render::TextureSampleDesc& Desc)
{
}

void CommandContext::SetShaderSampler(platform::Render::PixelHWShader* Shader, uint32 SamplerIndex, const platform::Render::TextureSampleDesc& Desc)
{
}

void CommandContext::SetShaderTexture(platform::Render::VertexHWShader* Shader, uint32 TextureIndex, platform::Render::Texture* Texture)
{
}

void CommandContext::SetShaderTexture(platform::Render::PixelHWShader* Shader, uint32 TextureIndex, platform::Render::Texture* Texture)
{
}

void CommandContext::SetShaderConstantBuffer(platform::Render::VertexHWShader* Shader, uint32 BaseIndex, platform::Render::GraphicsBuffer* Buffer)
{
}

void CommandContext::SetShaderConstantBuffer(platform::Render::PixelHWShader* Shader, uint32 BaseIndex, platform::Render::GraphicsBuffer* Buffer)
{
}

void CommandContext::SetShaderParameter(platform::Render::VertexHWShader* Shader, uint32 BufferIndex, uint32 BaseIndex, uint32 NumBytes, const void* NewValue)
{
}

void CommandContext::SetShaderParameter(platform::Render::PixelHWShader* Shader, uint32 BufferIndex, uint32 BaseIndex, uint32 NumBytes, const void* NewValue)
{
}

void CommandContext::DrawIndexPrimitive(platform::Render::GraphicsBuffer* IndexBuffer, int32 BaseVertexIndex, uint32 FirstInstance, uint32 NumVertices, uint32 StartIndex, uint32 NumPrimitives, uint32 NumInstances)
{
}

void CommandContext::DrawPrimitive(uint32 BaseVertexIndex, uint32 NumPrimitives, uint32 NumInstances)
{
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
			for (int32 TargetIndex = 0; TargetIndex < NumSimultaneousRTs; ++TargetIndex)
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
