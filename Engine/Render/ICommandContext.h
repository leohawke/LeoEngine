#pragma once

#include "IDevice.h"

namespace platform::Render {
	class ComputeContext
	{};

	//command context
	//On platforms that can processes command lists in parallel, it is a separate object.
	class CommandContext : public ComputeContext
	{
	public:
		virtual void BeginRenderPass(const RenderPassInfo& Info, const char* Name) = 0;

		virtual void SetViewport(uint32 MinX, uint32 MinY, float MinZ, uint32 MaxX, uint32 MaxY, float MaxZ) = 0;

		virtual void SetScissorRect(bool bEnable, uint32 MinX, uint32 MinY, uint32 MaxX, uint32 MaxY) = 0;

		virtual void SetVertexBuffer(uint32 slot, GraphicsBuffer* VertexBuffer) = 0;

		virtual void SetGraphicsPipelineState(GraphicsPipelineState* pso) = 0;

		virtual void SetShaderSampler(ShaderPass* Pass, ShaderCore::ShaderType Type, uint32 SamplerIndex,const TextureSampleDesc& Desc) = 0;

		virtual void SetShaderTexture(ShaderPass* Pass, ShaderCore::ShaderType Type, Texture* Texture) = 0;

		virtual void SetShaderConstantBuffer(ShaderPass* Pass, ShaderCore::ShaderType Type, GraphicsBuffer* Buffer) = 0;

		virtual void DrawIndexPrimitive(GraphicsBuffer* IndexBuffer, int32 BaseVertexIndex, uint32 FirstInstance, uint32 NumVertices, uint32 StartIndex, uint32 NumPrimitives, uint32 NumInstances) = 0;

		virtual void DrawPrimitive(uint32 BaseVertexIndex, uint32 NumPrimitives, uint32 NumInstances) = 0;
	};
}