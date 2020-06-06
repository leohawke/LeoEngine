#include "CommandContext.h"

using namespace platform_ex::Windows::D3D12;

void CommandContext::BeginRenderPass(const platform::Render::RenderPassInfo& Info, const char* Name)
{
}

void CommandContext::SetViewport(uint32 MinX, uint32 MinY, float MinZ, uint32 MaxX, uint32 MaxY, float MaxZ)
{
}

void CommandContext::SetScissorRect(bool bEnable, uint32 MinX, uint32 MinY, uint32 MaxX, uint32 MaxY)
{
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
