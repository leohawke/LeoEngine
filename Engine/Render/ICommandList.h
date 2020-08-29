#pragma once

#include "ICommandContext.h"

//Command List definitions for queueing up & executing later.

namespace platform::Render {
	class CommandListBase : leo::noncopyable
	{
	public:
		CommandListBase();

		void SetContext(CommandContext* InContext)
		{
			Context = InContext;
			ComputeContext = InContext;
		}

		CommandContext& GetContext()
		{
			return *Context;
		}

		ComputeContext& GetComputeContext()
		{
			return *ComputeContext;
		}

		void Reset();
	private:
		CommandContext* Context;
		ComputeContext* ComputeContext;
	};


	class CommandList :public CommandListBase
	{
	public:
		void BeginRenderPass(const RenderPassInfo& Info, const char* Name)
		{
			GetContext().BeginRenderPass(Info, Name);

			RenderTargetsInfo RTInfo(Info);
			
			CacheActiveRenderTargets(RTInfo.NumColorRenderTargets, RTInfo.ColorRenderTarget, &RTInfo.DepthStencilRenderTarget);
		}

		void SetViewport(uint32 MinX, uint32 MinY, float MinZ, uint32 MaxX, uint32 MaxY, float MaxZ)
		{
			GetContext().SetViewport(MinX, MinY, MinZ, MaxX, MaxY, MaxZ);
		}

		void SetScissorRect(bool bEnable, uint32 MinX, uint32 MinY, uint32 MaxX, uint32 MaxY)
		{
			GetContext().SetScissorRect(bEnable, MinX, MinY, MaxX, MaxY);
		}

		void SetVertexBuffer(uint32 slot, GraphicsBuffer* VertexBuffer)
		{
			GetContext().SetVertexBuffer(slot, VertexBuffer);
		}

		void SetGraphicsPipelineState(GraphicsPipelineState* pso)
		{
			GetContext().SetGraphicsPipelineState(pso);
		}

		template<typename THardwareShader>
		void SetShaderSampler(THardwareShader* Shader, uint32 SamplerIndex, const TextureSampleDesc& Desc)
		{
			GetContext().SetShaderSampler(Shader, SamplerIndex, Desc);
		}

		template<typename THardwareShader>
		void SetShaderTexture(THardwareShader* Shader, uint32 TextureIndex, Texture* Texture)
		{
			GetContext().SetShaderTexture(Shader, TextureIndex, Texture);
		}

		template<typename THardwareShader>
		void SetShaderResourceView(THardwareShader* Shader, uint32 TextureIndex, ShaderResourceView* SRV)
		{
			GetContext().SetShaderResourceView(Shader, TextureIndex, SRV);
		}

		template<typename THardwareShader>
		void SetShaderConstantBuffer(THardwareShader* Shader, uint32 BaseIndex, GraphicsBuffer* Buffer)
		{
			GetContext().SetShaderConstantBuffer(Shader, BaseIndex, Buffer);
		}

		void DrawIndexedPrimitive(GraphicsBuffer* IndexBuffer, int32 BaseVertexIndex, uint32 FirstInstance, uint32 NumVertices, uint32 StartIndex, uint32 NumPrimitives, uint32 NumInstances)
		{
			GetContext().DrawIndexedPrimitive(IndexBuffer, BaseVertexIndex, FirstInstance, NumVertices, StartIndex, NumPrimitives, NumInstances);
		}

		void DrawPrimitive(uint32 BaseVertexIndex, uint32 FirstInstance, uint32 NumPrimitives, uint32 NumInstances)
		{
			GetContext().DrawPrimitive(BaseVertexIndex, FirstInstance, NumPrimitives, NumInstances);
		}

		template<typename THardwareShader>
		void SetShaderParameter(THardwareShader* Shader, uint32 BufferIndex, uint32 BaseIndex, uint32 NumBytes, const void* NewValue)
		{
			GetContext().SetShaderParameter(Shader, BufferIndex, BaseIndex, NumBytes, NewValue);
		}

		void FillRenderTargetsInfo(GraphicsPipelineStateInitializer& GraphicsPSOInit)
		{
			GraphicsPSOInit.RenderTargetsEnabled = PSOContext.CachedNumSimultanousRenderTargets;
			for (uint32 i = 0; i < GraphicsPSOInit.RenderTargetsEnabled; ++i)
			{
				if (PSOContext.CachedRenderTargets[i].Texture)
				{
					GraphicsPSOInit.RenderTargetFormats[i] = PSOContext.CachedRenderTargets[i].Texture->GetFormat();
				}
				else
				{
					GraphicsPSOInit.RenderTargetFormats[i] = EF_Unknown;
				}

				if (GraphicsPSOInit.RenderTargetFormats[i] != EF_Unknown)
				{
					GraphicsPSOInit.NumSamples = PSOContext.CachedRenderTargets[i].Texture->GetSampleCount();
				}
			}

			if (PSOContext.CachedDepthStencilTarget.Texture)
			{
				GraphicsPSOInit.DepthStencilTargetFormat = PSOContext.CachedDepthStencilTarget.Texture->GetFormat();
			}
			else 
			{
				GraphicsPSOInit.DepthStencilTargetFormat = EF_Unknown;
			}
		}

		void SetUAVParameter(ComputeHWShader* Shader, uint32 UAVIndex, UnorderedAccessView* UAV)
		{
			GetComputeContext().SetUAVParameter(Shader, UAVIndex, UAV);
		}

		void SetUAVParameter(ComputeHWShader* Shader, uint32 UAVIndex, UnorderedAccessView* UAV, uint32 InitialCount)
		{
			GetComputeContext().SetUAVParameter(Shader, UAVIndex, UAV,InitialCount);
		}

		void SetComputeShader(ComputeHWShader* Shader)
		{
			GetComputeContext().SetComputeShader(Shader);
		}

		void DispatchComputShader(uint32 ThreadGroupCountX, uint32 ThreadGroupCountY, uint32 ThreadGroupCountZ)
		{
			GetComputeContext().DispatchComputeShader(ThreadGroupCountX, ThreadGroupCountY, ThreadGroupCountZ);
		}

		void BeginFrame();
		void EndFrame();
	protected:
		struct PSOContext
		{
			uint32 CachedNumSimultanousRenderTargets = 0;
			std::array<RenderTarget, MaxSimultaneousRenderTargets> CachedRenderTargets;
			DepthRenderTarget CachedDepthStencilTarget;

		} PSOContext;

		void CacheActiveRenderTargets(
			uint32 NewNumSimultaneousRenderTargets,
			const RenderTarget* RenderTargets,
			const DepthRenderTarget* DepthStencilTarget
		)
		{
			PSOContext.CachedNumSimultanousRenderTargets = NewNumSimultaneousRenderTargets;

			for (uint32 RTIdx = 0; RTIdx < PSOContext.CachedNumSimultanousRenderTargets; ++RTIdx)
			{
				PSOContext.CachedRenderTargets[RTIdx] = RenderTargets[RTIdx];
			}

			PSOContext.CachedDepthStencilTarget = DepthStencilTarget?*DepthStencilTarget: DepthRenderTarget();
		}
	};

	CommandList& GetCommandList();
}