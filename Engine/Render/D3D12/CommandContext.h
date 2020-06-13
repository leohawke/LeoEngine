#pragma once

#include "../ICommandContext.h"
#include "ContextStateCache.h"

namespace platform_ex::Windows::D3D12 {
	struct SetRenderTargetsInfo
	{
		RenderTargetView* ColorRenderTargets[platform::Render::MaxSimultaneousRenderTargets];
		int32 NumColorRenderTargets = 0;
		bool bClearColor = false;

		DepthStencilView* DepthStencilTarget = nullptr;
		bool bClearDepth = false;
		bool bClearStencil =false;

		UnorderedAccessView* UAVs[platform::Render::MaxSimultaneousUAVs];
		int32 NumUAVs = 0;

		void ConvertFromPassInfo(const platform::Render::RenderPassInfo& Info);
	};

	class CommandContext :public platform::Render::CommandContext
	{
	public:
		CommandContext(NodeDevice* InParent, SubAllocatedOnlineHeap::SubAllocationDesc& SubHeapDesc, bool InIsDefaultContext, bool InIsAsyncComputeContext = false);
	public:
		void BeginRenderPass(const platform::Render::RenderPassInfo& Info, const char* Name) override;

		void SetViewport(uint32 MinX, uint32 MinY, float MinZ, uint32 MaxX, uint32 MaxY, float MaxZ) override;

		void SetScissorRect(bool bEnable, uint32 MinX, uint32 MinY, uint32 MaxX, uint32 MaxY) override;

		void SetVertexBuffer(uint32 slot, platform::Render::GraphicsBuffer* VertexBuffer) override;

		void SetGraphicsPipelineState(platform::Render::GraphicsPipelineState* pso) override;

		void SetShaderSampler(platform::Render::VertexHWShader* Shader, uint32 SamplerIndex, const platform::Render::TextureSampleDesc& Desc) override;
		void SetShaderSampler(platform::Render::PixelHWShader* Shader, uint32 SamplerIndex, const platform::Render::TextureSampleDesc& Desc) override;

		void SetShaderTexture(platform::Render::VertexHWShader* Shader, uint32 TextureIndex, platform::Render::Texture* Texture) override;
		void SetShaderTexture(platform::Render::PixelHWShader* Shader, uint32 TextureIndex, platform::Render::Texture* Texture) override;

		void SetShaderConstantBuffer(platform::Render::VertexHWShader* Shader, uint32 BaseIndex, platform::Render::GraphicsBuffer* Buffer) override;
		void SetShaderConstantBuffer(platform::Render::PixelHWShader* Shader, uint32 BaseIndex, platform::Render::GraphicsBuffer* Buffer) override;

		void SetShaderParameter(platform::Render::VertexHWShader* Shader, uint32 BufferIndex, uint32 BaseIndex, uint32 NumBytes, const void* NewValue) override;
		void SetShaderParameter(platform::Render::PixelHWShader* Shader, uint32 BufferIndex, uint32 BaseIndex, uint32 NumBytes, const void* NewValue) override;

		void DrawIndexedPrimitive(platform::Render::GraphicsBuffer* IndexBuffer, int32 BaseVertexIndex, uint32 FirstInstance, uint32 NumVertices, uint32 StartIndex, uint32 NumPrimitives, uint32 NumInstances) override;

		void DrawPrimitive(uint32 BaseVertexIndex, uint32 NumPrimitives, uint32 NumInstances) override;

		void SetRenderTargets(
			uint32 NewNumSimultaneousRenderTargets,
			const RenderTargetView* const* NewRenderTargets,
			const DepthStencilView* NewDepthStencilTarget,
			uint32 NewNumUAVs,
			UnorderedAccessView* const* UAVs
		);

		void ClearMRT(bool bClearColor, int32 NumClearColors, const leo::math::float4* ColorArray, bool bClearDepth, float Depth, bool bClearStencil, uint32 Stencil);

		void SetRenderTargetsAndClear(const SetRenderTargetsInfo& RenderTargetsInfo);

		void OpenCommandList();
		void CloseCommandList();
	private:
		void CommitGraphicsResourceTables();
		void CommitNonComputeShaderConstants();
	public:
		FastConstantAllocator ConstantsAllocator;

		CommandContextStateCache StateCache;

		uint16 DirtyUniformBuffers[ShaderType::NumStandardType];

		ID3D12GraphicsCommandList* CommandListHandle;

		uint32 numDraws;

		/** Constant buffers for Set*ShaderParameter calls. */
		ConstantBuffer VSConstantBuffer;
		ConstantBuffer HSConstantBuffer;
		ConstantBuffer DSConstantBuffer;
		ConstantBuffer PSConstantBuffer;
		ConstantBuffer GSConstantBuffer;
		ConstantBuffer CSConstantBuffer;

		/** Track the currently bound constant buffers. */
		GraphicsBuffer* BoundConstantBuffers[ShaderType::NumStandardType][MAX_CBS];

		/** Bit array to track which uniform buffers have changed since the last draw call. */
		uint16 DirtyConstantBuffers[ShaderType::NumStandardType];
	private:
		bool bUsingTessellation = false;
		bool bDiscardSharedConstants = false;
	};
}