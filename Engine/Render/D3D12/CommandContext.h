#pragma once

#include "../ICommandContext.h"
#include "ContextStateCache.h"
#include "D3DCommandList.h"

namespace platform_ex::Windows::D3D12 {
	class CommandListManager;

	class Texture;

	class CommandContextBase :public platform::Render::CommandContext, public AdapterChild
	{
	public:
		CommandContextBase(D3D12Adapter* InParent, GPUMaskType InGPUMask, bool InIsDefaultContext, bool InIsAsyncComputeContext);

		bool IsDefaultContext() const { return bIsDefaultContext; }
	protected:
		const bool bIsDefaultContext;
		const bool  bIsAsyncComputeContext;
	};

	class CommandContext final :public CommandContextBase,public DeviceChild
	{
	public:
		CommandContext(NodeDevice* InParent, SubAllocatedOnlineHeap::SubAllocationDesc& SubHeapDesc, bool InIsDefaultContext, bool InIsAsyncComputeContext = false);
	public:
		void SetComputeShader(platform::Render::ComputeHWShader* ComputeShader) override;

		void DispatchComputeShader(uint32 ThreadGroupCountX, uint32 ThreadGroupCountY, uint32 ThreadGroupCountZ) override;

		void SetShaderTexture(platform::Render::ComputeHWShader* Shader, uint32 TextureIndex, platform::Render::Texture* Texture) override;

		void SetShaderSampler(platform::Render::ComputeHWShader* Shader, uint32 SamplerIndex, const platform::Render::TextureSampleDesc& Desc) override;

		void SetUAVParameter(platform::Render::ComputeHWShader* Shader, uint32 UAVIndex, platform::Render::UnorderedAccessView* UAV) override;

		void SetUAVParameter(platform::Render::ComputeHWShader* Shader, uint32 UAVIndex, platform::Render::UnorderedAccessView* UAV, uint32 InitialCount) override;

		void SetShaderParameter(platform::Render::ComputeHWShader* Shader, uint32 BufferIndex, uint32 BaseIndex, uint32 NumBytes, const void* NewValue) override;

		void SetComputePipelineState(platform::Render::ComputePipelineState* ComputeState) override;

		void BeginRenderPass(const platform::Render::RenderPassInfo& Info, const char* Name) override;

		void SetViewport(uint32 MinX, uint32 MinY, float MinZ, uint32 MaxX, uint32 MaxY, float MaxZ) override;

		void SetScissorRect(bool bEnable, uint32 MinX, uint32 MinY, uint32 MaxX, uint32 MaxY) override;

		void SetVertexBuffer(uint32 slot, platform::Render::GraphicsBuffer* VertexBuffer) override;

		void SetGraphicsPipelineState(platform::Render::GraphicsPipelineState* pso) override;

		void SetShaderSampler(platform::Render::VertexHWShader* Shader, uint32 SamplerIndex, const platform::Render::TextureSampleDesc& Desc) override;
		void SetShaderSampler(platform::Render::PixelHWShader* Shader, uint32 SamplerIndex, const platform::Render::TextureSampleDesc& Desc) override;

		void SetShaderTexture(platform::Render::VertexHWShader* Shader, uint32 TextureIndex, platform::Render::Texture* Texture) override;
		void SetShaderTexture(platform::Render::PixelHWShader* Shader, uint32 TextureIndex, platform::Render::Texture* Texture) override;

		void SetShaderResourceView(platform::Render::VertexHWShader* Shader, uint32 TextureIndex, platform::Render::ShaderResourceView* SRV) override;
		void SetShaderResourceView(platform::Render::PixelHWShader* Shader, uint32 TextureIndex, platform::Render::ShaderResourceView* SRV) override;

		void SetShaderConstantBuffer(platform::Render::VertexHWShader* Shader, uint32 BaseIndex, platform::Render::GraphicsBuffer* Buffer) override;
		void SetShaderConstantBuffer(platform::Render::PixelHWShader* Shader, uint32 BaseIndex, platform::Render::GraphicsBuffer* Buffer) override;

		void SetShaderParameter(platform::Render::VertexHWShader* Shader, uint32 BufferIndex, uint32 BaseIndex, uint32 NumBytes, const void* NewValue) override;
		void SetShaderParameter(platform::Render::PixelHWShader* Shader, uint32 BufferIndex, uint32 BaseIndex, uint32 NumBytes, const void* NewValue) override;

		void DrawIndexedPrimitive(platform::Render::GraphicsBuffer* IndexBuffer, int32 BaseVertexIndex, uint32 FirstInstance, uint32 NumVertices, uint32 StartIndex, uint32 NumPrimitives, uint32 NumInstances) override;

		void DrawPrimitive(uint32 BaseVertexIndex, uint32 FirstInstance, uint32 NumPrimitives, uint32 NumInstances) override;

		void SetRenderTargets(
			uint32 NewNumSimultaneousRenderTargets,
			const platform::Render::RenderTarget* NewRenderTargets, const platform::Render::DepthRenderTarget* NewDepthStencilTarget
		);

		void ClearMRT(bool bClearColor, int32 NumClearColors, const leo::math::float4* ColorArray, bool bClearDepth, float Depth, bool bClearStencil, uint32 Stencil);

		void SetRenderTargetsAndClear(const platform::Render::RenderTargetsInfo& RenderTargetsInfo);

		void OpenCommandList();
		void CloseCommandList();

		CommandListHandle FlushCommands(bool WaitForCompletion = false);

		void BeginFrame();

		void EndFrame();
	private:
		void ClearState();

		void CommitGraphicsResourceTables();
		void CommitNonComputeShaderConstants();

		void CommitComputeShaderConstants();
		void CommitComputeResourceTables(ComputeHWShader* ComputeShader);

		CommandListManager& GetCommandListManager();

		void ConditionalObtainCommandAllocator();

		void ReleaseCommandAllocator();

	public:
		FastConstantAllocator ConstantsAllocator;

		// Handles to the command list and direct command allocator this context owns (granted by the command list manager/command allocator manager), and a direct pointer to the D3D command list/command allocator.
		CommandListHandle CommandListHandle;
		CommandAllocator* CommandAllocator;

		CommandContextStateCache StateCache;

		// Tracks the currently set state blocks.
		RenderTargetView* CurrentRenderTargets[D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT];
		DepthStencilView* CurrentDepthStencilTarget;
		Texture* CurrentDepthTexture;
		uint32 NumSimultaneousRenderTargets;

		uint32 numDraws;
		uint32 numBarriers;

		bool HasDoneWork() const
		{
			return (numDraws+ numBarriers) > 0;
		}

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