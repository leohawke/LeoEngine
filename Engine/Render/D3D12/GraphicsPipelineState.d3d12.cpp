#include <LBase/ldef.h>
#include "GraphicsPipelineState.h"
#include "Convert.h"
#include "RootSignature.h"
#include "ShaderCompose.h"
#include "VertexDeclaration.h"
#include "Context.h"
#include <mutex>

using namespace platform_ex::Windows;
using namespace platform_ex::Windows::D3D12;

using platform::Render::GraphicsPipelineStateInitializer;

D3D12_GRAPHICS_PIPELINE_STATE_DESC D3D12::D3DGraphicsPipelineStateDesc::GraphicsDesc() const
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC D;
	D.Flags = this->Flags;
	D.NodeMask = this->NodeMask;
	D.pRootSignature = this->pRootSignature;
	D.InputLayout = this->InputLayout;
	D.IBStripCutValue = this->IBStripCutValue;
	D.PrimitiveTopologyType = this->PrimitiveTopologyType;
	D.VS = this->VS;
	D.GS = this->GS;
	D.HS = this->HS;
	D.DS = this->DS;
	D.PS = this->PS;
	D.BlendState = this->BlendState;
	D.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC1(this->DepthStencilState);
	D.DSVFormat = this->DSVFormat;
	D.RasterizerState = this->RasterizerState;
	D.NumRenderTargets = this->RTFormatArray.NumRenderTargets;
	std::memcpy(D.RTVFormats, this->RTFormatArray.RTFormats, sizeof(D.RTVFormats));
	std::memset(&D.StreamOutput, 0, sizeof(D.StreamOutput));
	D.SampleDesc = this->SampleDesc;
	D.SampleMask = this->SampleMask;
	D.CachedPSO = this->CachedPSO;
	return D;
}

D3D12_COMPUTE_PIPELINE_STATE_DESC D3DComputePipelineStateDesc::ComputeDescV0() const
{
	D3D12_COMPUTE_PIPELINE_STATE_DESC D = {};
	D.Flags = this->Flags;
	D.NodeMask = this->NodeMask;
	D.pRootSignature = this->pRootSignature;
	D.CS = this->CS;
	D.CachedPSO = this->CachedPSO;
	return D;
}

D3D12::D3DPipelineState::D3DPipelineState(D3D12Adapter* Parent)
	:AdapterChild(Parent), MultiNodeGPUObject(0, 0)
{
}

D3D12::D3DPipelineState::~D3DPipelineState()
{
}



GraphicsPipelineState::GraphicsPipelineState(const platform::Render::GraphicsPipelineStateInitializer& initializer, const D3D12::RootSignature* InRootSignature, D3DPipelineState* InPipelineState)
	:PipelineStateInitializer(initializer),
	RootSignature(InRootSignature),
	PipelineState(InPipelineState)
{
	std::memset(StreamStrides, 0, sizeof(StreamStrides));
	int index = 0;
	for (auto& stream : initializer.ShaderPass.VertexDeclaration)
	{
		StreamStrides[index++] = stream.Stride;
	}

	bShaderNeedsGlobalConstantBuffer[ShaderType::VertexShader] = GetVertexShader() && GetVertexShader()->bGlobalUniformBufferUsed;
	bShaderNeedsGlobalConstantBuffer[ShaderType::PixelShader] = GetPixelShader() && GetPixelShader()->bGlobalUniformBufferUsed;
	bShaderNeedsGlobalConstantBuffer[ShaderType::HullShader] = GetHullShader() && GetHullShader()->bGlobalUniformBufferUsed;
	bShaderNeedsGlobalConstantBuffer[ShaderType::DomainShader] = GetDomainShader() && GetDomainShader()->bGlobalUniformBufferUsed;
	bShaderNeedsGlobalConstantBuffer[ShaderType::GeometryShader] = GetGeometryShader() && GetGeometryShader()->bGlobalUniformBufferUsed;
}

static void TranslateRenderTargetFormats(
	const platform::Render::GraphicsPipelineStateInitializer& PsoInit,
	D3D12_RT_FORMAT_ARRAY& RTFormatArray,
	DXGI_FORMAT& DSVFormat
);

KeyGraphicsPipelineStateDesc D3D12::GetKeyGraphicsPipelineStateDesc(
	const platform::Render::GraphicsPipelineStateInitializer& initializer,const RootSignature* RootSignature)
{
	KeyGraphicsPipelineStateDesc Desc;
	std::memset(&Desc, 0, sizeof(Desc));

	Desc.pRootSignature = RootSignature;
	Desc.Desc.pRootSignature = RootSignature->GetSignature();

	Desc.Desc.BlendState = Convert(initializer.BlendState);
	Desc.Desc.SampleMask =initializer.BlendState.sample_mask;
	Desc.Desc.RasterizerState = Convert(initializer.RasterizerState);
	Desc.Desc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC1(Convert(initializer.DepthStencilState));

	if (false)
	{
		Desc.Desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
	}
	else
	{
		Desc.Desc.PrimitiveTopologyType = Convert<D3D12_PRIMITIVE_TOPOLOGY_TYPE>(initializer.Primitive);
	}

	TranslateRenderTargetFormats(initializer, Desc.Desc.RTFormatArray, Desc.Desc.DSVFormat);

	Desc.Desc.SampleDesc.Count = initializer.NumSamples;
	Desc.Desc.SampleDesc.Quality = initializer.NumSamples > DX_MAX_MSAA_COUNT ? 0XFFFFFFFF : 0;

	//InputLayout
	auto VertexDeclaration = CreateVertexDeclaration(initializer.ShaderPass.VertexDeclaration);
	if (VertexDeclaration)
	{
		Desc.Desc.InputLayout.NumElements = static_cast<UINT>(VertexDeclaration->VertexElements.size());
		Desc.Desc.InputLayout.pInputElementDescs = VertexDeclaration->VertexElements.data();
	}

	//CopyShader
#define COPY_SHADER(L,R) \
	if (D3D12HardwareShader* Shader = static_cast<R##HWShader*>(initializer.ShaderPass.R##Shader))\
	{\
		Desc.Desc.L##S << Shader;\
		Desc.L##SHash = Shader->GetHash();\
	}

	COPY_SHADER(V, Vertex);
	COPY_SHADER(P, Pixel);
	COPY_SHADER(D, Domain);
	COPY_SHADER(H, Hull);
	COPY_SHADER(G,Geometry)
#undef COPY_SHADER
	

	//don't support stream output

	return Desc;
}

KeyComputePipelineStateDesc GetKeyComputePipelineStateDesc(const ComputeHWShader* ComputeShader)
{
	KeyComputePipelineStateDesc Desc;
	std::memset(&Desc, 0, sizeof(Desc));

	Desc.pRootSignature = ComputeShader->pRootSignature;
	Desc.Desc.pRootSignature = Desc.pRootSignature->GetSignature();
	Desc.Desc.CS << ComputeShader;
	Desc.CSHash = ComputeShader->GetHash();

	return Desc;
}

static void TranslateRenderTargetFormats(
	const platform::Render::GraphicsPipelineStateInitializer& PsoInit,
	D3D12_RT_FORMAT_ARRAY& RTFormatArray,
	DXGI_FORMAT& DSVFormat
)
{
	RTFormatArray.NumRenderTargets = PsoInit.GetNumRenderTargets();

	for (uint32 RTIdx = 0; RTIdx < PsoInit.RenderTargetsEnabled; ++RTIdx)
	{
		DXGI_FORMAT PlatformFormat = PsoInit.RenderTargetFormats[RTIdx] != platform::Render::EF_Unknown ? Convert(PsoInit.RenderTargetFormats[RTIdx]) : DXGI_FORMAT_UNKNOWN;
	
		RTFormatArray.RTFormats[RTIdx] = PlatformFormat;
	}

	DXGI_FORMAT PlatformFormat = PsoInit.DepthStencilTargetFormat != platform::Render::EF_Unknown ? Convert(PsoInit.DepthStencilTargetFormat) : DXGI_FORMAT_UNKNOWN;

	DSVFormat = PlatformFormat;
}

void D3D12::QuantizeBoundShaderState(QuantizedBoundShaderState& QBSS, const platform::Render::GraphicsPipelineStateInitializer& initializer)
{
	std::memset(&QBSS, 0, sizeof(QBSS));

	QBSS.AllowIAInputLayout = !initializer.ShaderPass.VertexDeclaration.empty();

	auto Tier = GetDevice().GetResourceBindingTier();

	auto VertexShader = static_cast<VertexHWShader*>(initializer.ShaderPass.VertexShader);
	auto PixelShader = static_cast<PixelHWShader*>(initializer.ShaderPass.PixelShader);
	auto HullShader = static_cast<HullHWShader*>(initializer.ShaderPass.HullShader);
	auto DomainShader = static_cast<DomainHWShader*>(initializer.ShaderPass.DomainShader);
	auto GeometryShader = static_cast<GeometryHWShader*>(initializer.ShaderPass.GeometryShader);

	if (VertexShader)
		QuantizedBoundShaderState::InitShaderRegisterCounts(Tier, VertexShader->ResourceCounts, QBSS.RegisterCounts[ShaderType::VertexShader]);
	if (PixelShader)
		QuantizedBoundShaderState::InitShaderRegisterCounts(Tier, PixelShader->ResourceCounts, QBSS.RegisterCounts[ShaderType::PixelShader]);
	if (HullShader)
		QuantizedBoundShaderState::InitShaderRegisterCounts(Tier, HullShader->ResourceCounts, QBSS.RegisterCounts[ShaderType::HullShader]);
	if (DomainShader)
		QuantizedBoundShaderState::InitShaderRegisterCounts(Tier, DomainShader->ResourceCounts, QBSS.RegisterCounts[ShaderType::DomainShader]);
	if (GeometryShader)
		QuantizedBoundShaderState::InitShaderRegisterCounts(Tier, GeometryShader->ResourceCounts, QBSS.RegisterCounts[ShaderType::GeometryShader]);
}

uint64 D3DPipelineStateCacheBase::HashData(const void* Data, int32 NumBytes)
{
	return CityHash64((const char*)Data, NumBytes);
}

uint64 D3DPipelineStateCacheBase::HashPSODesc(const KeyGraphicsPipelineStateDesc& Desc)
{
	struct GraphicsPSOData
	{
		ShaderBytecodeHash VSHash;
		ShaderBytecodeHash HSHash;
		ShaderBytecodeHash DSHash;
		ShaderBytecodeHash GSHash;
		ShaderBytecodeHash PSHash;
		uint32 InputLayoutHash;

		uint8 AlphaToCoverageEnable;
		uint8 IndependentBlendEnable;

		uint32 SampleMask;
		D3D12_RASTERIZER_DESC RasterizerState;
		D3D12_DEPTH_STENCIL_DESC1 DepthStencilState;

		D3D12_INDEX_BUFFER_STRIP_CUT_VALUE IBStripCutValue;
		D3D12_PRIMITIVE_TOPOLOGY_TYPE PrimitiveTopologyType;
		DXGI_FORMAT DSVFormat;
		DXGI_SAMPLE_DESC SampleDesc;
		uint32 NodeMask;

		D3D12_PIPELINE_STATE_FLAGS Flags;
	};

	struct RenderTargetData
	{
		DXGI_FORMAT Format;
		D3D12_RENDER_TARGET_BLEND_DESC BlendDesc;
	};


	const int32 NumRenderTargets = Desc.Desc.RTFormatArray.NumRenderTargets;

	const size_t GraphicsPSODataSize = sizeof(GraphicsPSOData);
	const size_t RenderTargetDataSize = NumRenderTargets * sizeof(RenderTargetData);
	const size_t TotalDataSize = GraphicsPSODataSize + RenderTargetDataSize;

	char Data[GraphicsPSODataSize + 8 * sizeof(RenderTargetData)];
	std::memset(Data,0,TotalDataSize);

	GraphicsPSOData* PSOData = (GraphicsPSOData*)Data;
	RenderTargetData* RTData = (RenderTargetData*)(Data + GraphicsPSODataSize);

	PSOData->VSHash = Desc.VSHash;
	PSOData->HSHash = Desc.HSHash;
	PSOData->DSHash = Desc.DSHash;
	PSOData->GSHash = Desc.GSHash;
	PSOData->PSHash = Desc.PSHash;
	PSOData->InputLayoutHash = Desc.InputLayoutHash;

	PSOData->AlphaToCoverageEnable = Desc.Desc.BlendState.AlphaToCoverageEnable;
	PSOData->IndependentBlendEnable = Desc.Desc.BlendState.IndependentBlendEnable;
	PSOData->SampleMask = Desc.Desc.SampleMask;
	PSOData->RasterizerState = Desc.Desc.RasterizerState;
	PSOData->DepthStencilState = Desc.Desc.DepthStencilState;
	PSOData->IBStripCutValue = Desc.Desc.IBStripCutValue;
	PSOData->PrimitiveTopologyType = Desc.Desc.PrimitiveTopologyType;
	PSOData->DSVFormat = Desc.Desc.DSVFormat;
	PSOData->SampleDesc = Desc.Desc.SampleDesc;
	PSOData->NodeMask = Desc.Desc.NodeMask;
	PSOData->Flags = Desc.Desc.Flags;

	for (int32 RT = 0; RT < NumRenderTargets; RT++)
	{
		RTData[RT].Format = Desc.Desc.RTFormatArray.RTFormats[RT];
		RTData[RT].BlendDesc = Desc.Desc.BlendState.RenderTarget[RT];
	}

	return HashData(Data, static_cast<int32>(TotalDataSize));
}

uint64 D3DPipelineStateCacheBase::HashPSODesc(const KeyComputePipelineStateDesc& Desc)
{
	struct ComputePSOData
	{
		ShaderBytecodeHash CSHash;
		UINT NodeMask;
		D3D12_PIPELINE_STATE_FLAGS Flags;
	};
	ComputePSOData Data;
	std::memset(&Data,0,sizeof(Data));
	Data.CSHash = Desc.CSHash;
	Data.NodeMask = Desc.Desc.NodeMask;
	Data.Flags = Desc.Desc.Flags;

	return HashData(&Data, sizeof(Data));
}


D3DPipelineStateCacheBase::D3DPipelineStateCacheBase(D3D12Adapter* InParent)
	: AdapterChild(InParent)
{
}

D3DPipelineStateCacheBase::~D3DPipelineStateCacheBase()
{
	CleanupPipelineStateCaches();
}

void D3DPipelineStateCacheBase::CleanupPipelineStateCaches()
{
	{
		std::unique_lock Lock{ LowLevelGraphicsPipelineStateCacheMutex };
		for (auto& Pair : LowLevelGraphicsPipelineStateCache)
		{
			delete Pair.second;
		}
		LowLevelGraphicsPipelineStateCache.clear();
	}
}

D3DPipelineState* D3DPipelineStateCacheBase::FindInLowLevelCache(const KeyGraphicsPipelineStateDesc& Desc)
{
	lconstraint(Desc.CombinedHash != 0);

	{
		std::shared_lock Lock(LowLevelGraphicsPipelineStateCacheMutex);
		auto Found = LowLevelGraphicsPipelineStateCache.find(Desc);
		if (Found != LowLevelGraphicsPipelineStateCache.end())
		{
			return Found->second;
		}
	}

	return nullptr;
}

D3DPipelineState* D3DPipelineStateCacheBase::FindInLowLevelCache(const KeyComputePipelineStateDesc& Desc)
{
	lconstraint(Desc.CombinedHash != 0);

	{
		std::shared_lock Lock(ComputePipelineStateCacheMutex);
		auto Found = ComputePipelineStateCache.find(Desc);
		if (Found != ComputePipelineStateCache.end())
		{
			return Found->second;
		}
	}

	return nullptr;
}


D3DPipelineState* D3DPipelineStateCacheBase::CreateAndAddToLowLevelCache(const KeyGraphicsPipelineStateDesc& Desc)
{
	// Add PSO to low level cache.
	D3DPipelineState* PipelineState = nullptr;
	AddToLowLevelCache(Desc, &PipelineState, [this](D3DPipelineState** PipelineState, const KeyGraphicsPipelineStateDesc& Desc)
		{
			OnPSOCreated(*PipelineState, Desc);
		});

	return PipelineState;
}

D3DPipelineState* D3DPipelineStateCacheBase::CreateAndAddToLowLevelCache(const KeyComputePipelineStateDesc& Desc)
{
	// Add PSO to low level cache.
	D3DPipelineState* PipelineState = nullptr;
	AddToLowLevelCache(Desc, &PipelineState, [this](D3DPipelineState** PipelineState, const KeyComputePipelineStateDesc& Desc)
		{
			OnPSOCreated(*PipelineState, Desc);
		});

	return PipelineState;
}

void D3DPipelineStateCacheBase::AddToLowLevelCache(const KeyGraphicsPipelineStateDesc& Desc, D3DPipelineState** OutPipelineState, const FPostCreateGraphicCallback& PostCreateCallback)
{
	lconstraint(Desc.CombinedHash != 0);

	// Double check the desc doesn't already exist while the lock is taken.
	// This avoids having multiple threads try to create the same PSO.
	{
		std::unique_lock Lock(LowLevelGraphicsPipelineStateCacheMutex);
		auto PipelineState = LowLevelGraphicsPipelineStateCache.find(Desc);
		if (PipelineState != LowLevelGraphicsPipelineStateCache.end())
		{
			// This desc already exists.
			*OutPipelineState = PipelineState->second;
			return;
		}

		// Add the FD3D12PipelineState object to the cache, but don't actually create the underlying PSO yet while the lock is taken.
		// This allows multiple threads to create different PSOs at the same time.
		D3DPipelineState* NewPipelineState = new D3DPipelineState(GetParentAdapter());
		LowLevelGraphicsPipelineStateCache.emplace(Desc, NewPipelineState);

		*OutPipelineState = NewPipelineState;
	}

	// Create the underlying PSO and then perform any other additional tasks like cleaning up/adding to caches, etc.
	PostCreateCallback(OutPipelineState, Desc);
}

void D3DPipelineStateCacheBase::AddToLowLevelCache(const KeyComputePipelineStateDesc& Desc, D3DPipelineState** OutPipelineState, const FPostCreateComputeCallback& PostCreateCallback)
{
	lconstraint(Desc.CombinedHash != 0);

	// Double check the desc doesn't already exist while the lock is taken.
	// This avoids having multiple threads try to create the same PSO.
	{
		std::unique_lock Lock(ComputePipelineStateCacheMutex);
		auto PipelineState = ComputePipelineStateCache.find(Desc);
		if (PipelineState != ComputePipelineStateCache.end())
		{
			// This desc already exists.
			*OutPipelineState = PipelineState->second;
			return;
		}

		// Add the FD3D12PipelineState object to the cache, but don't actually create the underlying PSO yet while the lock is taken.
		// This allows multiple threads to create different PSOs at the same time.
		D3DPipelineState* NewPipelineState = new D3DPipelineState(GetParentAdapter());
		ComputePipelineStateCache.emplace(Desc, NewPipelineState);

		*OutPipelineState = NewPipelineState;
	}

	// Create the underlying PSO and then perform any other additional tasks like cleaning up/adding to caches, etc.
	PostCreateCallback(OutPipelineState, Desc);
}

GraphicsPipelineState* D3DPipelineStateCacheBase::FindInLoadedCache(
	const GraphicsPipelineStateInitializer& Initializer,
	const RootSignature* RootSignature,
	KeyGraphicsPipelineStateDesc& OutLowLevelDesc)
{
	// TODO: For now PSOs will be created on every node of the LDA chain.
	OutLowLevelDesc = GetKeyGraphicsPipelineStateDesc(Initializer, RootSignature);
	OutLowLevelDesc.Desc.NodeMask = 0;

	OutLowLevelDesc.CombinedHash = D3DPipelineStateCacheBase::HashPSODesc(OutLowLevelDesc);

	// First try to find the PSO in the low level cache that can be populated from disk.
	auto PipelineState = FindInLowLevelCache(OutLowLevelDesc);

	if (PipelineState && PipelineState->IsValid())
	{
		return new GraphicsPipelineState(Initializer, RootSignature, PipelineState);
	}

	return nullptr;
}

ComputePipelineState* D3DPipelineStateCacheBase::FindInLoadedCache(ComputeHWShader* ComputeShader, KeyComputePipelineStateDesc& OutLowLevelDesc)
{
	OutLowLevelDesc = GetKeyComputePipelineStateDesc(ComputeShader);
	OutLowLevelDesc.Desc.NodeMask = 0;
	OutLowLevelDesc.CombinedHash = D3DPipelineStateCacheBase::HashPSODesc(OutLowLevelDesc);

	// First try to find the PSO in the low level cache that can be populated from disk.
	auto PipelineState = FindInLowLevelCache(OutLowLevelDesc);

	if (PipelineState && PipelineState->IsValid())
	{
		return new ComputePipelineState(ComputeShader, PipelineState);
	}

	return nullptr;
}

GraphicsPipelineState* D3DPipelineStateCacheBase::CreateAndAdd(
	const GraphicsPipelineStateInitializer& Initializer,
	const RootSignature* RootSignature,
	const KeyGraphicsPipelineStateDesc& LowLevelDesc)
{
	auto const PipelineState = CreateAndAddToLowLevelCache(LowLevelDesc);

	if (PipelineState && PipelineState->IsValid())
	{
		return new GraphicsPipelineState(Initializer, RootSignature, PipelineState);
	}

	return nullptr;
}

ComputePipelineState* platform_ex::Windows::D3D12::D3DPipelineStateCacheBase::CreateAndAdd(ComputeHWShader* ComputeShader, const KeyComputePipelineStateDesc& LowLevelDesc)
{
	auto const PipelineState = CreateAndAddToLowLevelCache(LowLevelDesc);

	if (PipelineState && PipelineState->IsValid())
	{
		return new ComputePipelineState(ComputeShader, PipelineState);
	}

	return nullptr;
}

//Windows Only

void D3DPipelineStateCache::OnPSOCreated(D3DPipelineState* PipelineState, const KeyGraphicsPipelineStateDesc& Desc)
{
	const bool bAsync = /*!Desc.bFromPSOFileCache*/ false; // FORT-243931 - For now we need conclusive results of PSO creation succeess/failure synchronously to avoid PSO crashes

	// Actually create the PSO.
	GraphicsPipelineStateCreateArgs Args(&Desc, PipelineLibrary.Get());
	if (bAsync)
	{
		PipelineState->CreateAsync(Args);
	}
	else
	{
		PipelineState->Create(Args);
	}
}

void D3DPipelineStateCache::OnPSOCreated(D3DPipelineState* PipelineState, const KeyComputePipelineStateDesc& Desc)
{
	const bool bAsync = false;

	// Actually create the PSO.
	ComputePipelineCreationArgs Args(&Desc, PipelineLibrary.Get());
	if (bAsync)
	{
		PipelineState->CreateAsync(Args);
	}
	else
	{
		PipelineState->Create(Args);
	}
}

void D3DPipelineStateCache::Close()
{
	CleanupPipelineStateCaches();

	PipelineLibrary = nullptr;
}

void D3D12::D3DPipelineStateCache::Init(
	const std::string& GraphicsCacheFilename, 
	const std::string& ComputeCacheFilename, 
	const std::string& DriverBlobFilename)
{
	//Not using driver-optimized pipeline state disk cache
	bUseAPILibaries = false;
}

bool D3DPipelineStateCache::IsInErrorState() const
{
	return false;
}

D3DPipelineStateCache::D3DPipelineStateCache(D3D12Adapter* InParent)
	: D3DPipelineStateCacheBase(InParent)
	, bUseAPILibaries(true)
{
}

D3DPipelineStateCache::~D3DPipelineStateCache()
{
}

// Thread-safe create graphics/compute pipeline state. Conditionally load/store the PSO using a Pipeline Library.
static HRESULT CreatePipelineState(ID3D12PipelineState*& PSO, ID3D12Device* Device, const D3D12_GRAPHICS_PIPELINE_STATE_DESC* Desc, ID3D12PipelineLibrary* Library, const TCHAR* Name)
{
	HRESULT hr = S_OK;
	if (Library == nullptr || Library->LoadGraphicsPipeline(Name, Desc, IID_PPV_ARGS(&PSO)) == E_INVALIDARG)
	{
		hr = Device->CreateGraphicsPipelineState(Desc, IID_PPV_ARGS(&PSO));
	}

	if (Library && SUCCEEDED(hr))
	{
		HRESULT r = Library->StorePipeline(Name, PSO);
		lconstraint(r != E_INVALIDARG);
	}

	return hr;
}

static HRESULT CreatePipelineState(ID3D12PipelineState*& PSO, ID3D12Device* Device, const D3D12_COMPUTE_PIPELINE_STATE_DESC* Desc, ID3D12PipelineLibrary* Library, const TCHAR* Name)
{
	HRESULT hr = S_OK;
	if (Library == nullptr || Library->LoadComputePipeline(Name, Desc, IID_PPV_ARGS(&PSO)) == E_INVALIDARG)
	{
		hr = Device->CreateComputePipelineState(Desc, IID_PPV_ARGS(&PSO));
	}

	if (Library && SUCCEEDED(hr))
	{
		HRESULT r = Library->StorePipeline(Name, PSO);
		lconstraint(r != E_INVALIDARG);
	}

	return hr;
}

static inline void FastHashName(wchar_t Name[17], uint64 Hash)
{
	for (int32 i = 0; i < 16; i++)
	{
		Name[i] = (Hash & 0xF) + 'A';
		Hash >>= 4;
	}
	Name[16] = 0;
}

static void CreateGraphicsPipelineState(ID3D12PipelineState** PSO, D3D12Adapter* Adapter, const GraphicsPipelineStateCreateArgs* CreationArgs)
{
	// Get the pipeline state name, currently based on the hash.
	wchar_t Name[17];
	FastHashName(Name, CreationArgs->Desc->CombinedHash);

	const D3D12_GRAPHICS_PIPELINE_STATE_DESC Desc = CreationArgs->Desc->Desc.GraphicsDesc();
	HRESULT hr = CreatePipelineState(*PSO, Adapter->GetDevice(), &Desc, CreationArgs->Library, Name);
}

static void CreateComputePipelineState(ID3D12PipelineState** PSO, D3D12Adapter* Adapter, const ComputePipelineCreationArgs* CreationArgs)
{
	// Get the pipeline state name, currently based on the hash.
	wchar_t Name[17];
	FastHashName(Name, CreationArgs->Desc->CombinedHash);

	const auto Desc = CreationArgs->Desc->Desc.ComputeDescV0();
	HRESULT hr = CreatePipelineState(*PSO, Adapter->GetDevice(), &Desc, CreationArgs->Library, Name);
}


void D3D12::D3DPipelineState::Create(const GraphicsPipelineStateCreateArgs& InCreationArgs)
{
	CreateGraphicsPipelineState(PipelineState.ReleaseAndGetAddress(), GetParentAdapter(), &InCreationArgs);
}

void D3D12::D3DPipelineState::CreateAsync(const GraphicsPipelineStateCreateArgs& InCreationArgs)
{
	return Create(InCreationArgs);
}

void D3D12::D3DPipelineState::Create(const ComputePipelineCreationArgs& InCreationArgs)
{
	CreateComputePipelineState(PipelineState.ReleaseAndGetAddress(), GetParentAdapter(), &InCreationArgs);
}

void D3D12::D3DPipelineState::CreateAsync(const ComputePipelineCreationArgs& InCreationArgs)
{
	return Create(InCreationArgs);
}