#pragma once

#include "ICommandContext.h"

//Command List definitions for queueing up & executing later.

namespace platform::Render {
	class CommandListBase : leo::noncopyable
	{
	public:
		void SetContext(CommandContext* InContext)
		{
			Context = InContext;
		}

		CommandContext& GetContext()
		{
			return *Context;
		}
	private:
		CommandContext* Context;
	};

	class CommandList :public CommandListBase
	{
	public:
		void BeginRenderPass(const RenderPassInfo& Info, const char* Name)
		{
			GetContext().BeginRenderPass(Info, Name);

			const RenderTargetView* ColorRenderTargets[MaxSimultaneousRenderTargets];
			uint32 NumColorRenderTargets = 0;
			for (int32 Index = 0; Index < MaxSimultaneousRenderTargets; ++Index)
			{
				if (!Info.ColorRenderTargets[Index])
				{
					break;
				}
				ColorRenderTargets[Index] = Info.ColorRenderTargets[Index];
				++NumColorRenderTargets;
			}

			CacheActiveRenderTargets(NumColorRenderTargets, ColorRenderTargets, Info.DepthStencilTarget);
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

		void SetShaderSampler(ShaderPass* Pass, Shader::ShaderType Type, uint32 SamplerIndex, const TextureSampleDesc& Desc)
		{
			GetContext().SetShaderSampler(Pass, Type, SamplerIndex, Desc);
		}

		void SetShaderTexture(ShaderPass* Pass, Shader::ShaderType Type, uint32 TextureIndex, Texture* Texture)
		{
			GetContext().SetShaderTexture(Pass, Type, TextureIndex, Texture);
		}

		void SetShaderConstantBuffer(ShaderPass* Pass, Shader::ShaderType Type, uint32 BaseIndex, GraphicsBuffer* Buffer)
		{
			GetContext().SetShaderConstantBuffer(Pass, Type, BaseIndex, Buffer);
		}

		void DrawIndexPrimitive(GraphicsBuffer* IndexBuffer, int32 BaseVertexIndex, uint32 FirstInstance, uint32 NumVertices, uint32 StartIndex, uint32 NumPrimitives, uint32 NumInstances)
		{
			GetContext().DrawIndexPrimitive(IndexBuffer, BaseVertexIndex, FirstInstance, NumVertices, StartIndex, NumPrimitives, NumInstances);
		}

		void DrawPrimitive(uint32 BaseVertexIndex, uint32 NumPrimitives, uint32 NumInstances)
		{
			GetContext().DrawPrimitive(BaseVertexIndex, NumPrimitives, NumInstances);
		}


		void FillRenderTargetsInfo(GraphicsPipelineStateInitializer& GraphicsPSOInit)
		{
			GraphicsPSOInit.RenderTargetsEnabled = PSOContext.CachedNumSimultanousRenderTargets;
			for (uint32 i = 0; i < GraphicsPSOInit.RenderTargetsEnabled; ++i)
			{
				if (PSOContext.CachedRenderTargets[i])
				{
					GraphicsPSOInit.RenderTargetFormats[i] = PSOContext.CachedRenderTargets[i]->Format();
				}
				else
				{
					GraphicsPSOInit.RenderTargetFormats[i] = EF_Unknown;
				}

				if (GraphicsPSOInit.RenderTargetFormats[i] != EF_Unknown)
				{
					GraphicsPSOInit.NumSamples = PSOContext.CachedRenderTargets[i]->GetNumSamples();
				}
			}

			if (PSOContext.CachedDepthStencilTarget)
			{
				GraphicsPSOInit.DepthStencilTargetFormat = PSOContext.CachedDepthStencilTarget->Format();
			}
			else 
			{
				GraphicsPSOInit.DepthStencilTargetFormat = EF_Unknown;
			}
		}
	protected:
		struct PSOContext
		{
			uint32 CachedNumSimultanousRenderTargets = 0;
			std::array<const RenderTargetView*, MaxSimultaneousRenderTargets> CachedRenderTargets;
			const DepthStencilView* CachedDepthStencilTarget;

		} PSOContext;

		void CacheActiveRenderTargets(
			uint32 NewNumSimultaneousRenderTargets,
			const RenderTargetView** RenderTargets,
			const DepthStencilView* DepthStencilTarget
		)
		{
			PSOContext.CachedNumSimultanousRenderTargets = NewNumSimultaneousRenderTargets;

			for (uint32 RTIdx = 0; RTIdx < PSOContext.CachedNumSimultanousRenderTargets; ++RTIdx)
			{
				PSOContext.CachedRenderTargets[RTIdx] = RenderTargets[RTIdx];
			}

			PSOContext.CachedDepthStencilTarget = DepthStencilTarget ;
		}
	};

	CommandList& GetCommandList();
}