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


	class CommandList : CommandListBase
	{
	public:
		void BeginRenderPass(const RenderPassInfo& Info, const char* Name)
		{

		}

		void SetViewport(uint32 MinX, uint32 MinY, float MinZ, uint32 MaxX, uint32 MaxY, float MaxZ);

		void SetScissorRect(bool bEnable, uint32 MinX, uint32 MinY, uint32 MaxX, uint32 MaxY);

		void SetVertexBuffer(uint32 slot, GraphicsBuffer* VertexBuffer);

		void SetGraphicsPipelineState(GraphicsPipelineState* pso);

		void SetShaderSampler(ShaderPass* Pass, ShaderCore::ShaderType Type, uint32 SamplerIndex, const TextureSampleDesc& Desc);

		void SetShaderTexture(ShaderPass* Pass, ShaderCore::ShaderType Type, Texture* Texture);

		void SetShaderConstantBuffer(ShaderPass* Pass, ShaderCore::ShaderType Type, GraphicsBuffer* Buffer);

		void DrawIndexPrimitive(GraphicsBuffer* IndexBuffer, int32 BaseVertexIndex, uint32 FirstInstance, uint32 NumVertices, uint32 StartIndex, uint32 NumPrimitives, uint32 NumInstances);

		void DrawPrimitive(uint32 BaseVertexIndex, uint32 NumPrimitives, uint32 NumInstances);


		void FillRenderTargetsInfo(GraphicsPipelineStateInitializer& InPsoInit)
		{

		}
	};
}