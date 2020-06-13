#pragma once

#include <LBase/observer_ptr.hpp>

#include "../ShaderCore.h"
#include "../PipleState.h"
#include "ResourceView.h"
#include "Common.h"
#include "GraphicsPipelineState.h"
#include "ResourceHolder.h"
#include "HardwareShader.h"
#include "ConstantBuffer.h"
#include "Convert.h"
#include "DescriptorCache.h"

// Device Context State
// improve draw thread performance by removing redundant device context calls.
// TODO:refcount resource

namespace platform_ex::Windows::D3D12 {

	using platform::Render::Shader::ShaderType;

	constexpr auto NUM_SAMPLER_DESCRIPTORS = D3D12_MAX_SHADER_VISIBLE_SAMPLER_HEAP_SIZE;
	constexpr auto DESCRIPTOR_HEAP_BLOCK_SIZE = 10000;

	constexpr auto NUM_VIEW_DESCRIPTORS_TIER_1 = D3D12_MAX_SHADER_VISIBLE_DESCRIPTOR_HEAP_SIZE_TIER_1;
	constexpr auto NUM_VIEW_DESCRIPTORS_TIER_2 = D3D12_MAX_SHADER_VISIBLE_DESCRIPTOR_HEAP_SIZE_TIER_2;
	// Only some tier 3 hardware can use > 1 million descriptors in a heap, the only way to tell if hardware can
	// is to try and create a heap and check for failure. Unless we really want > 1 million Descriptors we'll cap
	// out at 1M for now.
	constexpr auto NUM_VIEW_DESCRIPTORS_TIER_3 = D3D12_MAX_SHADER_VISIBLE_DESCRIPTOR_HEAP_SIZE_TIER_2;

	class GraphicsBuffer;

	enum CachePipelineType
	{
		CPT_Graphics,
		//TODO
		CPT_Compute,
	};

	constexpr unsigned int max_vbs = D3D12_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT;

	using VBSlotMask = leo::make_width_int<max_vbs>::unsigned_fast_type;

	struct VertexBufferCache
	{
		VertexBufferCache()
		{
			Clear();
		}

		void Clear()
		{
			std::memset(CurrentVertexBufferResources, 0, sizeof(CurrentVertexBufferResources));
			std::memset(CurrentVertexBufferViews, 0, sizeof(CurrentVertexBufferViews));
			MaxBoundVertexBufferIndex = -1;
			BoundVBMask = 0;
		}

		GraphicsBuffer* CurrentVertexBufferResources[max_vbs];
		D3D12_VERTEX_BUFFER_VIEW CurrentVertexBufferViews[max_vbs];
		int32 MaxBoundVertexBufferIndex;
		VBSlotMask BoundVBMask;
	};

	struct IndexBufferCache
	{
		IndexBufferCache()
		{
			Clear();
		}

		void Clear()
		{
			std::memset(&CurrentIndexBufferView, 0, sizeof(CurrentIndexBufferView));
		}

		D3D12_INDEX_BUFFER_VIEW CurrentIndexBufferView;
	};

	template<typename SlotMaskType>
	struct ResourceSlotMask
	{
		static inline void CleanSlot(SlotMaskType& SlotMask, uint32 SlotIndex)
		{
			SlotMask &= ~((SlotMaskType)1 << SlotIndex);
		}

		static inline void CleanSlots(SlotMaskType& SlotMask, uint32 NumSlots)
		{
			SlotMask &= ~(((SlotMaskType)1 << NumSlots) - 1);
		}

		static inline void DirtySlot(SlotMaskType& SlotMask, uint32 SlotIndex)
		{
			SlotMask |= ((SlotMaskType)1 << SlotIndex);
		}

		static inline bool IsSlotDirty(const SlotMaskType& SlotMask, uint32 SlotIndex)
		{
			return (SlotMask & ((SlotMaskType)1 << SlotIndex)) != 0;
		}

		// Mark a specific shader stage as dirty.
		inline void Dirty(ShaderType ShaderFrequency, const SlotMaskType & SlotMask = -1)
		{
			DirtySlotMask[ShaderFrequency] |= SlotMask;
		}

		// Mark specified bind slots, on all graphics stages, as dirty.
		inline void DirtyGraphics(const SlotMaskType& SlotMask = -1)
		{
			Dirty(ShaderType::VertexShader, SlotMask);
			Dirty(ShaderType::PixelShader, SlotMask);
			Dirty(ShaderType::HullShader, SlotMask);
			Dirty(ShaderType::DomainShader, SlotMask);
			Dirty(ShaderType::GeometryShader, SlotMask);
		}

		// Mark specified bind slots on compute as dirty.
		inline void DirtyCompute(const SlotMaskType& SlotMask = -1)
		{
			Dirty(ShaderType::ComputeShader, SlotMask);
		}

		// Mark specified bind slots on graphics and compute as dirty.
		inline void DirtyAll(const SlotMaskType& SlotMask = -1)
		{
			DirtyGraphics(SlotMask);
			DirtyCompute(SlotMask);
		}

		SlotMaskType DirtySlotMask[ShaderType::NumStandardType];
	};

	struct ConstantBufferCache : public ResourceSlotMask<CBVSlotMask>
	{
		ConstantBufferCache()
		{
			Clear();
		}

		void Clear()
		{
			DirtyAll();

			std::memset(CurrentGPUVirtualAddress, 0, sizeof(CurrentGPUVirtualAddress));
		}

		D3D12_GPU_VIRTUAL_ADDRESS CurrentGPUVirtualAddress[ShaderType::NumStandardType][MAX_CBS];
	};

	struct ShaderResourceViewCache : public ResourceSlotMask<SRVSlotMask>
	{
		ShaderResourceViewCache()
		{
			Clear();
		}

		void Clear()
		{
			DirtyAll();

			std::memset(Views, 0, sizeof(Views));

			std::memset(BoundMask, 0, sizeof(BoundMask));

			for (auto& Index : MaxBoundIndex)
			{
				Index = -1;
			}
		}

		ShaderResourceView* Views[ShaderType::NumStandardType][MAX_SRVS];
		SRVSlotMask BoundMask[ShaderType::NumStandardType];
		int32 MaxBoundIndex[ShaderType::NumStandardType];
	};

	struct UnorderedAccessViewCache : public ResourceSlotMask<UAVSlotMask>
	{
		UnorderedAccessViewCache()
		{
			Clear();
		}

		void Clear()
		{
			DirtyAll();

			std::memset(Views, 0, sizeof(Views));

			for (auto& Index : StartSlot)
			{
				Index = -1;
			}
		}

		UnorderedAccessView* Views[ShaderType::NumStandardType][MAX_UAVS];
		uint32 StartSlot[ShaderType::NumStandardType];
	};

	struct SamplerStateCache :public ResourceSlotMask<SamplerSlotMask>
	{
		SamplerStateCache()
		{
			Clear();
		}

		void Clear()
		{
			DirtyAll();

			std::memset(States, 0, sizeof(States));
		}

		platform::Render::TextureSampleDesc States[ShaderType::NumStandardType][MAX_SAMPLERS];
	};

	class CommandContext;

	using D3D12DescriptorCache = DescriptorCache;

	class CommandContextStateCache : public DeviceChild, public SingleNodeGPUObject
	{
	protected:
		CommandContext* CmdContext;

		bool bNeedSetVB;
		bool bNeedSetIB;
		bool bNeedSetRTs;
		bool bNeedSetSOs;
		bool bSRVSCleared;
		bool bNeedSetViewports;
		bool bNeedSetScissorRects;
		bool bNeedSetPrimitiveTopology;
		bool bNeedSetBlendFactor;
		bool bNeedSetStencilRef;
		bool bNeedSetDepthBounds;
		bool bAutoFlushComputeShaderCache;
		D3D12_RESOURCE_BINDING_TIER ResourceBindingTier;

		struct
		{
			struct
			{
				// Cache
				leo::observer_ptr<GraphicsPipelineState> CurrentPipelineStateObject;

				// Note: Current root signature is part of the bound shader state, which is part of the PSO
				bool bNeedSetRootSignature;

				// Depth Stencil State Cache
				uint32 CurrentReferenceStencil;

				// Blend State Cache
				float CurrentBlendFactor[4];

				// Viewport
				uint32	CurrentNumberOfViewports;
				D3D12_VIEWPORT CurrentViewport[D3D12_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];

				// Vertex Buffer State
				VertexBufferCache VBCache;

				// SO
				uint32			CurrentNumberOfStreamOutTargets;
				ResourceHolder* CurrentStreamOutTargets[D3D12_SO_STREAM_COUNT];
				uint32			CurrentSOOffsets[D3D12_SO_STREAM_COUNT];

				// Index Buffer State
				IndexBufferCache IBCache;

				// Primitive Topology State
				platform::Render::PrimtivteType CurrentPrimitiveType;
				D3D_PRIMITIVE_TOPOLOGY CurrentPrimitiveTopology;

				// Input Layout State
				D3D12_RECT CurrentScissorRects[D3D12_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];
				uint32 CurrentNumberOfScissorRects;

				uint16 StreamStrides[platform::Render::MaxVertexElementCount];

				RenderTargetView* RenderTargetArray[D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT];
				uint32 CurrentNumberOfRenderTargets;

				DepthStencilView* CurrentDepthStencilTarget;

				float MinDepth;
				float MaxDepth;
			} Graphics;

			struct
			{
				ShaderResourceViewCache SRVCache;
				ConstantBufferCache CBVCache;
				UnorderedAccessViewCache UAVCache;
				SamplerStateCache SamplerCache;

				// PSO
				ID3D12PipelineState* CurrentPipelineStateObject;
				bool bNeedSetPSO;

				uint32 CurrentShaderSamplerCounts[ShaderType::NumStandardType];
				uint32 CurrentShaderSRVCounts[ShaderType::NumStandardType];
				uint32 CurrentShaderCBCounts[ShaderType::NumStandardType];
				uint32 CurrentShaderUAVCounts[ShaderType::NumStandardType];
			} Common;
		} PipelineState;

		D3D12DescriptorCache DescriptorCache;

		void InternalSetIndexBuffer(GraphicsBuffer* IndexBufferLocation, DXGI_FORMAT Format, uint32 Offset);

		void InternalSetStreamSource(GraphicsBuffer* VertexBufferLocation, uint32 StreamIndex, uint32 Stride, uint32 Offset);

		template <typename TShader> struct StateCacheShaderTraits;
#define DECLARE_SHADER_TRAITS(Name) \
	template <> struct StateCacheShaderTraits<Name##HWShader> \
	{ \
		static const ShaderType Frequency = ShaderType::Name##Shader; \
		static Name##HWShader* GetShader(GraphicsPipelineState* PSO) { return PSO ? (Name##HWShader*)PSO->PipelineStateInitializer.ShaderPass.##Name##Shader : nullptr; } \
	}
		DECLARE_SHADER_TRAITS(Vertex);
		DECLARE_SHADER_TRAITS(Pixel);
		DECLARE_SHADER_TRAITS(Domain);
		DECLARE_SHADER_TRAITS(Hull);
		DECLARE_SHADER_TRAITS(Geometry);
#undef DECLARE_SHADER_TRAITS

		template <typename TShader> void SetShader(TShader* Shader)
		{
			typedef StateCacheShaderTraits<TShader> Traits;
			TShader* OldShader = Traits::GetShader(GetGraphicsPipelineState());

			if (OldShader != Shader)
			{
				PipelineState.Common.CurrentShaderSamplerCounts[Traits::Frequency] = (Shader) ? Shader->ResourceCounts.NumSamplers : 0;
				PipelineState.Common.CurrentShaderSRVCounts[Traits::Frequency] = (Shader) ? Shader->ResourceCounts.NumSRVs : 0;
				PipelineState.Common.CurrentShaderCBCounts[Traits::Frequency] = (Shader) ? Shader->ResourceCounts.NumCBs : 0;
				PipelineState.Common.CurrentShaderUAVCounts[Traits::Frequency] = (Shader) ? Shader->ResourceCounts.NumUAVs : 0;

				// Shader changed so its resource table is dirty
				this->CmdContext->DirtyUniformBuffers[Traits::Frequency] = 0xffff;
			}
		}

		template <typename TShader> void GetShader(TShader** Shader)
		{
			*Shader = StateCacheShaderTraits<TShader>::GetShader(GetGraphicsPipelineState());
		}

		template <CachePipelineType PipelineType>
		void InternalSetPipelineState()
		{
			static_assert(PipelineType != CPT_Compute, "CommandContextStateCache is not support to be used with compute.");

			// See if we need to set our PSO:
			// In D3D11, you could Set dispatch arguments, then set Draw arguments, then call Draw/Dispatch/Draw/Dispatch without setting arguments again.
			// In D3D12, we need to understand when the app switches between Draw/Dispatch and make sure the correct PSO is set.

			bool bNeedSetPSO = PipelineState.Common.bNeedSetPSO;
			ID3D12PipelineState*& CurrentPSO = PipelineState.Common.CurrentPipelineStateObject;
			ID3D12PipelineState* const RequiredPSO = (PipelineType == CPT_Compute)
				? nullptr
				: PipelineState.Graphics.CurrentPipelineStateObject->GetPipelineState();

			if (CurrentPSO != RequiredPSO)
			{
				CurrentPSO = RequiredPSO;
				bNeedSetPSO = true;
			}

			// Set the PSO on the command list if necessary.
			if (bNeedSetPSO)
			{
				this->CmdContext->CommandListHandle->SetPipelineState(CurrentPSO);
				PipelineState.Common.bNeedSetPSO = false;
			}
		}


	public:

		void InheritState(const CommandContextStateCache& AncestralCache)
		{
			std::memcpy(&PipelineState, &AncestralCache.PipelineState, sizeof(PipelineState));
			DirtyState();
		}

		D3D12DescriptorCache* GetDescriptorCache()
		{
			return &DescriptorCache;
		}

		GraphicsPipelineState* GetGraphicsPipelineState() const
		{
			return PipelineState.Graphics.CurrentPipelineStateObject.get();
		}

		const RootSignature* GetGraphicsRootSignature()
		{
			return PipelineState.Graphics.CurrentPipelineStateObject ? PipelineState.Graphics.CurrentPipelineStateObject->RootSignature : nullptr;
		}

		inline platform::Render::PrimtivteType GetPrimtivteType() const
		{
			return PipelineState.Graphics.CurrentPrimitiveType;
		}

		void ClearSRVs();

		template <ShaderType ShaderFrequency>
		void ClearShaderResourceViews(ResourceHolder*& ResourceLocation)
		{
			//SCOPE_CYCLE_COUNTER(STAT_D3D12ClearShaderResourceViewsTime);

			if (PipelineState.Common.SRVCache.MaxBoundIndex[ShaderFrequency] < 0)
			{
				return;
			}

			auto& CurrentShaderResourceViews = PipelineState.Common.SRVCache.Views[ShaderFrequency];
			for (int32 i = 0; i <= PipelineState.Common.SRVCache.MaxBoundIndex[ShaderFrequency]; ++i)
			{
				if (CurrentShaderResourceViews[i] && CurrentShaderResourceViews[i]->GetResourceHolder() == ResourceLocation)
				{
					SetShaderResourceView<ShaderFrequency>(nullptr, i);
				}
			}
		}

		template <ShaderType ShaderFrequency>
		void SetShaderResourceView(ShaderResourceView* SRV, uint32 ResourceIndex);

		void SetScissorRects(uint32 Count, const D3D12_RECT* const ScissorRects);
		void SetScissorRect(const D3D12_RECT& ScissorRect);

		const D3D12_RECT& GetScissorRect(int32 Index = 0) const
		{
			return PipelineState.Graphics.CurrentScissorRects[Index];
		}

		void SetViewport(const D3D12_VIEWPORT& Viewport);
		void SetViewports(uint32 Count, const D3D12_VIEWPORT* const Viewports);

		uint32 GetNumViewports() const
		{
			return PipelineState.Graphics.CurrentNumberOfViewports;
		}

		const D3D12_VIEWPORT& GetViewport(int32 Index = 0) const
		{
			return PipelineState.Graphics.CurrentViewport[Index];
		}

		void GetViewports(uint32* Count, D3D12_VIEWPORT* Viewports) const
		{
			if (Viewports) //NULL is legal if you just want count
			{
				//as per d3d spec
				const int32 StorageSizeCount = (int32)(*Count);
				const int32 CopyCount = std::min(std::min(StorageSizeCount, (int32)PipelineState.Graphics.CurrentNumberOfViewports), D3D12_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE);
				if (CopyCount > 0)
				{
					std::memcpy(Viewports, &PipelineState.Graphics.CurrentViewport[0], sizeof(D3D12_VIEWPORT) * CopyCount);
				}
				//remaining viewports in supplied array must be set to zero
				if (StorageSizeCount > CopyCount)
				{
					std::memset(&Viewports[CopyCount], 0, sizeof(D3D12_VIEWPORT) * (StorageSizeCount - CopyCount));
				}
			}
			*Count = PipelineState.Graphics.CurrentNumberOfViewports;
		}

		template <ShaderType ShaderFrequency>
		void SetSamplerState(const platform::Render::TextureSampleDesc& SamplerState, uint32 SamplerIndex)
		{
			lconstraint(SamplerIndex < MAX_SAMPLERS);
			auto& Samplers = PipelineState.Common.SamplerCache.States[ShaderFrequency];
			if ((Samplers[SamplerIndex] != SamplerState))
			{
				Samplers[SamplerIndex] = SamplerState;
				SamplerStateCache::DirtySlot(PipelineState.Common.SamplerCache.DirtySlotMask[ShaderFrequency], SamplerIndex);
			}
		}

		template <ShaderType ShaderFrequency>
		void GetSamplerState(uint32 StartSamplerIndex, uint32 NumSamplerIndexes, platform::Render::TextureSampleDesc* SamplerStates) const
		{
			auto& CurrentShaderResourceViews = PipelineState.Common.SRVCache.Views[ShaderFrequency];
			for (uint32 StateLoop = 0; StateLoop < NumSamplerIndexes; StateLoop++)
			{
				SamplerStates[StateLoop] = CurrentShaderResourceViews[StateLoop + StartSamplerIndex];
			}
		}

		template <ShaderType ShaderFrequency>
		void SetConstantsBuffer(uint32 SlotIndex, GraphicsBuffer* UniformBuffer)
		{
			auto& CBVCache = PipelineState.Common.CBVCache;
			D3D12_GPU_VIRTUAL_ADDRESS& CurrentGPUVirtualAddress = CBVCache.CurrentGPUVirtualAddress[ShaderFrequency][SlotIndex];

			if (UniformBuffer && UniformBuffer->Resource()->GetGPUVirtualAddress())
			{
				const auto& ResourceLocation = UniformBuffer->Resource();
				// Only update the constant buffer if it has changed.
				if (ResourceLocation->GetGPUVirtualAddress() != CurrentGPUVirtualAddress)
				{
					CurrentGPUVirtualAddress = ResourceLocation->GetGPUVirtualAddress();
					ConstantBufferCache::DirtySlot(CBVCache.DirtySlotMask[ShaderFrequency], SlotIndex);
				}
			}
			else if (CurrentGPUVirtualAddress != 0)
			{
				CurrentGPUVirtualAddress = 0;
				ConstantBufferCache::DirtySlot(CBVCache.DirtySlotMask[ShaderFrequency], SlotIndex);
			}
			else
			{
			}
		}

		template <ShaderType ShaderFrequency>
		void SetConstantBuffer(ConstantBuffer& Buffer, bool bDiscardSharedConstants)
		{
			ID3D12Resource* Location = nullptr;
			if (Buffer.Version(Location, bDiscardSharedConstants))
			{
				// Note: Code assumes the slot index is always 0.
				const uint32 SlotIndex = 0;

				auto& CBVCache = PipelineState.Common.CBVCache;
				D3D12_GPU_VIRTUAL_ADDRESS& CurrentGPUVirtualAddress = CBVCache.CurrentGPUVirtualAddress[ShaderFrequency][SlotIndex];
				CurrentGPUVirtualAddress = Location->GetGPUVirtualAddress();
				ConstantBufferCache::DirtySlot(CBVCache.DirtySlotMask[ShaderFrequency], SlotIndex);
			}
		}

		void SetBlendFactor(const float BlendFactor[4]);
		const float* GetBlendFactor() const { return PipelineState.Graphics.CurrentBlendFactor; }

		void SetStencilRef(uint32 StencilRef);
		uint32 GetStencilRef() const { return PipelineState.Graphics.CurrentReferenceStencil; }

		void GetVertexShader(VertexHWShader** Shader)
		{
			GetShader(Shader);
		}

		void GetHullShader(HullHWShader** Shader)
		{
			GetShader(Shader);
		}

		void GetDomainShader(DomainHWShader** Shader)
		{
			GetShader(Shader);
		}

		void GetGeometryShader(GeometryHWShader** Shader)
		{
			GetShader(Shader);
		}

		void GetPixelShader(PixelHWShader** Shader)
		{
			GetShader(Shader);
		}

		void SetGraphicsPipelineState(GraphicsPipelineState* GraphicsPipelineState, bool bTessellationChanged)
		{
			if (PipelineState.Graphics.CurrentPipelineStateObject.get() != GraphicsPipelineState)
			{
				SetStreamStrides(GraphicsPipelineState->StreamStrides);
				SetShader(GraphicsPipelineState->GetVertexShader());
				SetShader(GraphicsPipelineState->GetPixelShader());
				SetShader(GraphicsPipelineState->GetDomainShader());
				SetShader(GraphicsPipelineState->GetHullShader());
				SetShader(GraphicsPipelineState->GetGeometryShader());
				// See if we need to change the root signature
				if (GetGraphicsRootSignature() != GraphicsPipelineState->RootSignature)
				{
					PipelineState.Graphics.bNeedSetRootSignature = true;
				}

				// Save the PSO
				PipelineState.Common.bNeedSetPSO = true;
				PipelineState.Graphics.CurrentPipelineStateObject.reset(GraphicsPipelineState);

				auto PrimitiveType = GraphicsPipelineState->PipelineStateInitializer.Primitive;
				if (PipelineState.Graphics.CurrentPrimitiveType != PrimitiveType || bTessellationChanged)
				{
					const bool bUsingTessellation = GraphicsPipelineState->GetHullShader() && GraphicsPipelineState->GetDomainShader();
					PipelineState.Graphics.CurrentPrimitiveType = PrimitiveType;
					PipelineState.Graphics.CurrentPrimitiveTopology = Convert<D3D_PRIMITIVE_TOPOLOGY>(PrimitiveType);
					bNeedSetPrimitiveTopology = true;
				}

				// Set the PSO
				InternalSetPipelineState<CPT_Graphics>();
			}
		}

		void SetStreamStrides(const uint16* InStreamStrides)
		{
			std::memcpy(PipelineState.Graphics.StreamStrides, InStreamStrides, sizeof(PipelineState.Graphics.StreamStrides));
		}

		void SetStreamSource(GraphicsBuffer* VertexBufferLocation, uint32 StreamIndex, uint32 Stride, uint32 Offset)
		{
			InternalSetStreamSource(VertexBufferLocation, StreamIndex, Stride, Offset);
		}

		void SetStreamSource(GraphicsBuffer* VertexBufferLocation, uint32 StreamIndex, uint32 Offset)
		{
			InternalSetStreamSource(VertexBufferLocation, StreamIndex, PipelineState.Graphics.StreamStrides[StreamIndex], Offset);
		}

		bool IsShaderResource(const ResourceHolder* VertexBufferLocation) const
		{
			for (int i = 0; i < ShaderType::NumStandardType; i++)
			{
				if (PipelineState.Common.SRVCache.MaxBoundIndex[i] < 0)
				{
					continue;
				}

				for (int32 j = 0; j < PipelineState.Common.SRVCache.MaxBoundIndex[i]; ++j)
				{
					if (PipelineState.Common.SRVCache.Views[i][j] && PipelineState.Common.SRVCache.Views[i][j]->GetResourceHolder())
					{
						if (PipelineState.Common.SRVCache.Views[i][j]->GetResourceHolder() == VertexBufferLocation)
						{
							return true;
						}
					}
				}
			}

			return false;
		}

		bool IsStreamSource(const ResourceHolder* VertexBufferLocation) const
		{
			for (int32 index = 0; index <= PipelineState.Graphics.VBCache.MaxBoundVertexBufferIndex; ++index)
			{
				if (PipelineState.Graphics.VBCache.CurrentVertexBufferViews[index].BufferLocation == VertexBufferLocation->Resource()->GetGPUVirtualAddress())
				{
					return true;
				}
			}

			return false;
		}

	public:

		void SetIndexBuffer(GraphicsBuffer* IndexBufferLocation, DXGI_FORMAT Format, uint32 Offset)
		{
			InternalSetIndexBuffer(IndexBufferLocation, Format, Offset);
		}

		bool IsIndexBuffer(const ResourceHolder* ResourceLocation) const
		{
			return PipelineState.Graphics.IBCache.CurrentIndexBufferView.BufferLocation == ResourceLocation->Resource()->GetGPUVirtualAddress();
		}

		void GetPrimitiveTopology(D3D12_PRIMITIVE_TOPOLOGY* PrimitiveTopology) const
		{
			*PrimitiveTopology = PipelineState.Graphics.CurrentPrimitiveTopology;
		}

		CommandContextStateCache(GPUMaskType Node);

		void Init(NodeDevice* InParent, CommandContext* InCmdContext, const CommandContextStateCache* AncestralState,SubAllocatedOnlineHeap::SubAllocationDesc& SubHeapDesc);

		virtual ~CommandContextStateCache()
		{
		}

		template <CachePipelineType PipelineType>
		void ApplyState();
		void ApplySamplers(const RootSignature* const pRootSignature, uint32 StartStage, uint32 EndStage);
		void DirtyStateForNewCommandList();
		void DirtyState();
		void DirtyViewDescriptorTables();
		void DirtySamplerDescriptorTables();
		bool AssertResourceStates(CachePipelineType PipelineType);

		void SetRenderTargets(uint32 NumSimultaneousRenderTargets,const RenderTargetView* const* RTArray,const DepthStencilView* DSTarget);
		void GetRenderTargets(RenderTargetView** RTArray, uint32* NumSimultaneousRTs, DepthStencilView** DepthStencilTarget)
		{
			if (RTArray) //NULL is legal
			{
				std::memcpy(RTArray, PipelineState.Graphics.RenderTargetArray, sizeof(RenderTargetView*) * D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT);
				*NumSimultaneousRTs = PipelineState.Graphics.CurrentNumberOfRenderTargets;
			}

			if (DepthStencilTarget)
			{
				*DepthStencilTarget = PipelineState.Graphics.CurrentDepthStencilTarget;
			}
		}

		template <ShaderType ShaderStage>
		void SetUAVs(uint32 UAVStartSlot, uint32 NumSimultaneousUAVs, UnorderedAccessView* const* UAVArray, uint32* UAVInitialCountArray);
		template <ShaderType ShaderStage>
		void ClearUAVs();

		void SetDepthBounds(float MinDepth, float MaxDepth)
		{
			if (PipelineState.Graphics.MinDepth != MinDepth || PipelineState.Graphics.MaxDepth != MaxDepth)
			{
				PipelineState.Graphics.MinDepth = MinDepth;
				PipelineState.Graphics.MaxDepth = MaxDepth;

				bNeedSetDepthBounds = true;
			}
		}

		/**
		 * Clears all D3D12 State, setting all input/output resource slots, shaders, input layouts,
		 * predications, scissor rectangles, depth-stencil state, rasterizer state, blend state,
		 * sampler state, and viewports to NULL
		 */
		virtual void ClearState();

		/**
		 * Releases any object references held by the state cache
		 */
		void Clear();

		void ForceSetGraphicsRootSignature() { PipelineState.Graphics.bNeedSetRootSignature = true; }
		void ForceSetVB() { bNeedSetVB = true; }
		void ForceSetIB() { bNeedSetIB = true; }
		void ForceSetRTs() { bNeedSetRTs = true; }
		void ForceSetSOs() { bNeedSetSOs = true; }
		void ForceSetSamplersPerShaderStage(uint32 Frequency) { PipelineState.Common.SamplerCache.Dirty((ShaderType)Frequency); }
		void ForceSetSRVsPerShaderStage(uint32 Frequency) { PipelineState.Common.SRVCache.Dirty((ShaderType)Frequency); }
		void ForceSetViewports() { bNeedSetViewports = true; }
		void ForceSetScissorRects() { bNeedSetScissorRects = true; }
		void ForceSetPrimitiveTopology() { bNeedSetPrimitiveTopology = true; }
		void ForceSetBlendFactor() { bNeedSetBlendFactor = true; }
		void ForceSetStencilRef() { bNeedSetStencilRef = true; }

		bool GetForceSetVB() const { return bNeedSetVB; }
		bool GetForceSetIB() const { return bNeedSetIB; }
		bool GetForceSetRTs() const { return bNeedSetRTs; }
		bool GetForceSetSOs() const { return bNeedSetSOs; }
		bool GetForceSetSamplersPerShaderStage(uint32 Frequency) const { return PipelineState.Common.SamplerCache.DirtySlotMask[Frequency] != 0; }
		bool GetForceSetSRVsPerShaderStage(uint32 Frequency) const { return PipelineState.Common.SRVCache.DirtySlotMask[Frequency] != 0; }
		bool GetForceSetViewports() const { return bNeedSetViewports; }
		bool GetForceSetScissorRects() const { return bNeedSetScissorRects; }
		bool GetForceSetPrimitiveTopology() const { return bNeedSetPrimitiveTopology; }
		bool GetForceSetBlendFactor() const { return bNeedSetBlendFactor; }
		bool GetForceSetStencilRef() const { return bNeedSetStencilRef; }
	};
}