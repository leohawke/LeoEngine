#pragma once

#include <LFramework/Win32/LCLib/COM.h>
#include "../IGraphicsPipelineState.h"
#include "../IDevice.h"
#include "../ShaderCore.h"
#include "d3d12_dxgi.h"
#include "Common.h"
#include "Utility.h"
#include <shared_mutex>
#include <unordered_map>

namespace platform_ex::Windows::D3D12
{
	class RootSignature;
	struct QuantizedBoundShaderState;

	struct D3DGraphicsPipelineStateDesc
	{
		ID3D12RootSignature* pRootSignature;
		D3D12_SHADER_BYTECODE VS;
		D3D12_SHADER_BYTECODE PS;
		D3D12_SHADER_BYTECODE DS;
		D3D12_SHADER_BYTECODE HS;
		D3D12_SHADER_BYTECODE GS;

		D3D12_BLEND_DESC BlendState;
		uint32 SampleMask;
		D3D12_RASTERIZER_DESC RasterizerState;
		D3D12_DEPTH_STENCIL_DESC1 DepthStencilState;

		D3D12_INPUT_LAYOUT_DESC InputLayout;
		D3D12_INDEX_BUFFER_STRIP_CUT_VALUE IBStripCutValue;
		D3D12_PRIMITIVE_TOPOLOGY_TYPE PrimitiveTopologyType;
		D3D12_RT_FORMAT_ARRAY RTFormatArray;
		DXGI_FORMAT DSVFormat;
		DXGI_SAMPLE_DESC SampleDesc;
		uint32 NodeMask;
		D3D12_CACHED_PIPELINE_STATE CachedPSO;
		D3D12_PIPELINE_STATE_FLAGS Flags;

		D3D12_GRAPHICS_PIPELINE_STATE_DESC GraphicsDesc() const;
	};

	struct KeyGraphicsPipelineStateDesc
	{
		const RootSignature* pRootSignature;
		D3DGraphicsPipelineStateDesc Desc;

		ShaderBytecodeHash VSHash;
		ShaderBytecodeHash HSHash;
		ShaderBytecodeHash DSHash;
		ShaderBytecodeHash GSHash;
		ShaderBytecodeHash PSHash;
		uint32 InputLayoutHash;
		bool bFromPSOFileCache;

		SIZE_T CombinedHash;

		std::string GetName() const {
			return leo::sfmt("%llu", CombinedHash);
		}
	};

	template <typename TDesc> struct equality_pipeline_state_desc;

	template <> struct equality_pipeline_state_desc<KeyGraphicsPipelineStateDesc>
	{
#define PSO_IF_NOT_EQUAL_RETURN_FALSE( value ) if(lhs.##value != rhs.##value){ return false; }
#define PSO_IF_MEMCMP_FAILS_RETURN_FALSE( value ) if(std::memcmp(&lhs.##value, &rhs.##value, sizeof(rhs.##value)) != 0){ return false; }
#define PSO_IF_STRING_COMPARE_FAILS_RETURN_FALSE( value ) \
	const char* const lhString = lhs.##value##; \
	const char* const rhString = rhs.##value##; \
	if (lhString != rhString) \
	{ \
		if (strcmp(lhString, rhString) != 0) \
		{ \
			return false; \
		} \
	}
		bool operator()(const KeyGraphicsPipelineStateDesc& lhs, const KeyGraphicsPipelineStateDesc& rhs)
		{
			// Order from most likely to change to least
			PSO_IF_NOT_EQUAL_RETURN_FALSE(Desc.PS.BytecodeLength)
				PSO_IF_NOT_EQUAL_RETURN_FALSE(Desc.VS.BytecodeLength)
				PSO_IF_NOT_EQUAL_RETURN_FALSE(Desc.GS.BytecodeLength)
				PSO_IF_NOT_EQUAL_RETURN_FALSE(Desc.DS.BytecodeLength)
				PSO_IF_NOT_EQUAL_RETURN_FALSE(Desc.HS.BytecodeLength)
				PSO_IF_NOT_EQUAL_RETURN_FALSE(Desc.InputLayout.NumElements)
				PSO_IF_NOT_EQUAL_RETURN_FALSE(Desc.RTFormatArray.NumRenderTargets)
				PSO_IF_NOT_EQUAL_RETURN_FALSE(Desc.DSVFormat)
				PSO_IF_NOT_EQUAL_RETURN_FALSE(Desc.PrimitiveTopologyType)
				PSO_IF_NOT_EQUAL_RETURN_FALSE(Desc.Flags)
				PSO_IF_NOT_EQUAL_RETURN_FALSE(Desc.pRootSignature)
				PSO_IF_MEMCMP_FAILS_RETURN_FALSE(Desc.BlendState)
				PSO_IF_NOT_EQUAL_RETURN_FALSE(Desc.SampleMask)
				PSO_IF_MEMCMP_FAILS_RETURN_FALSE(Desc.RasterizerState)
				PSO_IF_MEMCMP_FAILS_RETURN_FALSE(Desc.DepthStencilState)
				PSO_IF_NOT_EQUAL_RETURN_FALSE(Desc.IBStripCutValue)
				PSO_IF_NOT_EQUAL_RETURN_FALSE(Desc.NodeMask)
				PSO_IF_NOT_EQUAL_RETURN_FALSE(Desc.SampleDesc.Count)
				PSO_IF_NOT_EQUAL_RETURN_FALSE(Desc.SampleDesc.Quality)

				for (uint32 i = 0; i < lhs.Desc.RTFormatArray.NumRenderTargets; i++)
				{
					PSO_IF_NOT_EQUAL_RETURN_FALSE(Desc.RTFormatArray.RTFormats[i]);
				}

			// Shader byte code is hashed with SHA1 (160 bit) so the chances of collision
			// should be tiny i.e if there were 1 quadrillion shaders the chance of a 
			// collision is ~ 1 in 10^18. so only do a full check on debug builds
			PSO_IF_NOT_EQUAL_RETURN_FALSE(VSHash)
				PSO_IF_NOT_EQUAL_RETURN_FALSE(PSHash)
				PSO_IF_NOT_EQUAL_RETURN_FALSE(GSHash)
				PSO_IF_NOT_EQUAL_RETURN_FALSE(HSHash)
				PSO_IF_NOT_EQUAL_RETURN_FALSE(DSHash)

				if (lhs.Desc.InputLayout.pInputElementDescs != rhs.Desc.InputLayout.pInputElementDescs &&
					lhs.Desc.InputLayout.NumElements)
				{
					for (uint32 i = 0; i < lhs.Desc.InputLayout.NumElements; i++)
					{
						PSO_IF_NOT_EQUAL_RETURN_FALSE(Desc.InputLayout.pInputElementDescs[i].SemanticIndex)
							PSO_IF_NOT_EQUAL_RETURN_FALSE(Desc.InputLayout.pInputElementDescs[i].Format)
							PSO_IF_NOT_EQUAL_RETURN_FALSE(Desc.InputLayout.pInputElementDescs[i].InputSlot)
							PSO_IF_NOT_EQUAL_RETURN_FALSE(Desc.InputLayout.pInputElementDescs[i].AlignedByteOffset)
							PSO_IF_NOT_EQUAL_RETURN_FALSE(Desc.InputLayout.pInputElementDescs[i].InputSlotClass)
							PSO_IF_NOT_EQUAL_RETURN_FALSE(Desc.InputLayout.pInputElementDescs[i].InstanceDataStepRate)
							PSO_IF_STRING_COMPARE_FAILS_RETURN_FALSE(Desc.InputLayout.pInputElementDescs[i].SemanticName)
					}
				}
			return true;
		}
#undef PSO_IF_NOT_EQUAL_RETURN_FALSE
#undef PSO_IF_MEMCMP_FAILS_RETURN_FALSE
#undef PSO_IF_STRING_COMPARE_FAILS_RETURN_FALSE
	};

	struct GraphicsPipelineStateCreateArgs;

	//Async Support

	struct D3DPipelineState : public AdapterChild, public MultiNodeGPUObject
	{
	public:
		explicit D3DPipelineState(D3D12Adapter* Parent);
		~D3DPipelineState();

		void Create(const GraphicsPipelineStateCreateArgs& InCreationArgs);
		void CreateAsync(const GraphicsPipelineStateCreateArgs& InCreationArgs);

		bool IsValid()
		{
			return (GetPipelineState() != nullptr);
		}

		ID3D12PipelineState* GetPipelineState() const
		{
			return PipelineState.Get();
		}

		D3DPipelineState& operator=(const D3DPipelineState& other) = delete;
	private:
		KeyGraphicsPipelineStateDesc Key;

		COMPtr<ID3D12PipelineState> PipelineState;
	};

	class GraphicsPipelineState : public platform::Render::GraphicsPipelineState
	{
	public:
		explicit GraphicsPipelineState(const platform::Render::GraphicsPipelineStateInitializer& initializer, const RootSignature* InRootSignature, D3DPipelineState* InPipelineState);

		ID3D12PipelineState* GetPipelineState() const
		{
			return PipelineState->GetPipelineState();
		}
	public:
		const platform::Render::GraphicsPipelineStateInitializer PipelineStateInitializer;
		const RootSignature* RootSignature;
		uint16 StreamStrides[platform::Render:: MaxVertexElementCount];

		bool bShaderNeedsGlobalConstantBuffer[platform::Render::ShaderType::NumStandardType];

		D3DPipelineState* PipelineState;

		class VertexHWShader* GetVertexShader() const { return (VertexHWShader*)PipelineStateInitializer.ShaderPass.VertexShader; }
		class PixelHWShader* GetPixelShader() const { return (PixelHWShader*)PipelineStateInitializer.ShaderPass.PixelShader; }
		class HullHWShader* GetHullShader() const { return (HullHWShader*)PipelineStateInitializer.ShaderPass.HullShader; }
		class DomainHWShader* GetDomainShader() const { return (DomainHWShader*)PipelineStateInitializer.ShaderPass.DomainShader; }
		class GeometryHWShader* GetGeometryShader() const { return (GeometryHWShader*)PipelineStateInitializer.ShaderPass.GeometryShader;; }
	private:
		KeyGraphicsPipelineStateDesc Key;
	};

	KeyGraphicsPipelineStateDesc GetKeyGraphicsPipelineStateDesc(
		const platform::Render::GraphicsPipelineStateInitializer& initializer, const RootSignature* RootSignature);

	void QuantizeBoundShaderState(QuantizedBoundShaderState& QBSS, const platform::Render::GraphicsPipelineStateInitializer& initializer);

	struct GraphicsPipelineStateCreateArgs
	{
		const KeyGraphicsPipelineStateDesc* Desc;
		ID3D12PipelineLibrary* Library;

		GraphicsPipelineStateCreateArgs(const KeyGraphicsPipelineStateDesc* InDesc, ID3D12PipelineLibrary* InLibrary)
		{
			Desc = InDesc;
			Library = InLibrary;
		}
	};

	class D3DPipelineStateCacheBase : public AdapterChild
	{
	protected:
		enum PSO_CACHE_TYPE
		{
			PSO_CACHE_GRAPHICS,
			PSO_CACHE_COMPUTE,
			NUM_PSO_CACHE_TYPES
		};

		template <typename TDesc, typename TValue>
		struct TStateCacheKeyFuncs
		{
			typedef TDesc KeyInitType;

			bool operator()(const KeyInitType& A,const KeyInitType& B) const
			{
				equality_pipeline_state_desc<TDesc> equal;
				return equal(A, B);
			}

			std::size_t operator()(const KeyInitType& Key) const
			{
				return Key.CombinedHash;
			}
		};

		template <typename TDesc, typename TValue = D3DPipelineState*>
		using TPipelineCache = std::unordered_map<TDesc, TValue, TStateCacheKeyFuncs<TDesc, TValue>,TStateCacheKeyFuncs<TDesc, TValue>>;

		TPipelineCache<KeyGraphicsPipelineStateDesc> LowLevelGraphicsPipelineStateCache;

		// Thread access mutual exclusion
		mutable std::shared_mutex LowLevelGraphicsPipelineStateCacheMutex;
		mutable std::shared_mutex ComputePipelineStateCacheMutex;

		void CleanupPipelineStateCaches();

		typedef std::function<void(D3DPipelineState**, const KeyGraphicsPipelineStateDesc&)> FPostCreateGraphicCallback;

		D3DPipelineState* FindInLowLevelCache(const KeyGraphicsPipelineStateDesc& Desc);
		D3DPipelineState* CreateAndAddToLowLevelCache(const KeyGraphicsPipelineStateDesc& Desc);
		void AddToLowLevelCache(const KeyGraphicsPipelineStateDesc& Desc, D3DPipelineState** OutPipelineState, const FPostCreateGraphicCallback& PostCreateCallback);
		virtual void OnPSOCreated(D3DPipelineState* PipelineState, const KeyGraphicsPipelineStateDesc& Desc) = 0;

		GraphicsPipelineState* FindInLoadedCache(const platform::Render::GraphicsPipelineStateInitializer& Initializer, const RootSignature* RootSignature, KeyGraphicsPipelineStateDesc& OutLowLevelDesc);
		GraphicsPipelineState* CreateAndAdd(const platform::Render::GraphicsPipelineStateInitializer& Initializer, const RootSignature* RootSignature, const KeyGraphicsPipelineStateDesc& LowLevelDesc);
	public:
		static uint64 HashPSODesc(const KeyGraphicsPipelineStateDesc& Desc);

		static uint64 HashData(const void* Data, int32 NumBytes);

		D3DPipelineStateCacheBase(D3D12Adapter* InParent);
		virtual ~D3DPipelineStateCacheBase();
	};

	//Windows Only
	class D3DPipelineStateCache : public D3DPipelineStateCacheBase
	{
	private:
		COMPtr<ID3D12PipelineLibrary> PipelineLibrary;
		bool bUseAPILibaries;

		bool UsePipelineLibrary() const
		{
			return bUseAPILibaries && PipelineLibrary != nullptr;
		}

		bool UseCachedBlobs() const
		{
			// Use Cached Blobs if Pipeline Librarys aren't supported.
			//return bUseAPILibaries && !UsePipelineLibrary();
			return false; // Don't try to use cached blobs (for now).
		}

	protected:

		void OnPSOCreated(D3DPipelineState* PipelineState, const KeyGraphicsPipelineStateDesc& Desc) final override;

	public:
		using D3DPipelineStateCacheBase::FindInLoadedCache;
		using D3DPipelineStateCacheBase::CreateAndAdd;

		void Close();

		void Init(const std::string& GraphicsCacheFilename,const std::string& ComputeCacheFilename,const std::string& DriverBlobFilename);
		bool IsInErrorState() const;

		D3DPipelineStateCache(D3D12Adapter* InParent);
		virtual ~D3DPipelineStateCache();
	};
}