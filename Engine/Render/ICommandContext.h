#pragma once

#include "IDevice.h"
#include "RenderPassInfo.h"

namespace platform::Render {
	class ComputeContext
	{
	public:
		/**
		*Sets the current compute shader.
		*/
		virtual void SetComputeShader(ComputeHWShader* ComputeShader) = 0;

		virtual void SetComputePipelineState(ComputePipelineState* ComputeState) = 0;

		virtual void DispatchComputeShader(uint32 ThreadGroupCountX, uint32 ThreadGroupCountY, uint32 ThreadGroupCountZ) = 0;

		virtual void SetShaderTexture(ComputeHWShader* Shader, uint32 TextureIndex, Texture* Texture) = 0;

		virtual void SetShaderSampler(ComputeHWShader* Shader, uint32 SamplerIndex, const TextureSampleDesc& Desc) = 0;

		virtual void SetUAVParameter(ComputeHWShader* Shader, uint32 UAVIndex, UnorderedAccessView* UAV) = 0;

		virtual void SetUAVParameter(ComputeHWShader* Shader, uint32 UAVIndex, UnorderedAccessView* UAV, uint32 InitialCount) = 0;

		virtual void SetShaderParameter(ComputeHWShader* Shader, uint32 BufferIndex, uint32 BaseIndex, uint32 NumBytes, const void* NewValue) = 0;

		virtual void PushEvent(const char16_t* Name, platform::FColor Color) = 0;

		virtual void PopEvent() = 0;
	};

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

		virtual void SetShaderSampler(VertexHWShader* Shader, uint32 SamplerIndex, const TextureSampleDesc& Desc) = 0;
		virtual void SetShaderSampler(PixelHWShader* Shader, uint32 SamplerIndex,const TextureSampleDesc& Desc) = 0;

		virtual void SetShaderTexture(VertexHWShader* Shader, uint32 TextureIndex, Texture* Texture) = 0;
		virtual void SetShaderTexture(PixelHWShader* Shader,uint32 TextureIndex,Texture* Texture) = 0;
		virtual void SetShaderResourceView(VertexHWShader* Shader, uint32 TextureIndex, ShaderResourceView* SRV) = 0;
		virtual void SetShaderResourceView(PixelHWShader* Shader, uint32 TextureIndex, ShaderResourceView* SRV) = 0;

		virtual void SetShaderConstantBuffer(VertexHWShader* Shader, uint32 BaseIndex, GraphicsBuffer* Buffer) = 0;
		virtual void SetShaderConstantBuffer(PixelHWShader* Shader,uint32 BaseIndex, GraphicsBuffer* Buffer) = 0;

		virtual void SetShaderParameter(VertexHWShader* Shader, uint32 BufferIndex, uint32 BaseIndex, uint32 NumBytes, const void* NewValue) = 0;
		virtual void SetShaderParameter(PixelHWShader* Shader, uint32 BufferIndex, uint32 BaseIndex, uint32 NumBytes, const void* NewValue) = 0;

		virtual void DrawIndexedPrimitive(GraphicsBuffer* IndexBuffer, int32 BaseVertexIndex, uint32 FirstInstance, uint32 NumVertices, uint32 StartIndex, uint32 NumPrimitives, uint32 NumInstances) = 0;

		virtual void DrawPrimitive(uint32 BaseVertexIndex, uint32 FirstInstance, uint32 NumPrimitives, uint32 NumInstances) = 0;

		virtual void BeginFrame() = 0;

		virtual void EndFrame() = 0;

	};
}