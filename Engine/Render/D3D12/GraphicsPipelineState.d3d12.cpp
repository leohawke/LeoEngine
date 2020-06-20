#include <LBase/ldef.h>
#include "GraphicsPipelineState.h"
#include "Convert.h"
#include "RootSignature.h"
#include "ShaderCompose.h"
#include "VertexDeclaration.h"
#include "Context.h"

using namespace platform_ex::Windows::D3D12;

inline void operator<<(D3D12_SHADER_BYTECODE& desc, const platform::Render::HardwareShader* pShader)
{
	auto pD3DShader = dynamic_cast<const D3D12HardwareShader*>(pShader);
	if (!pD3DShader)
		return;

	desc.BytecodeLength = pD3DShader->ShaderByteCode.second;
	desc.pShaderBytecode = pD3DShader->ShaderByteCode.first.get();
}

void QuantizeBoundShaderState(QuantizedBoundShaderState& QBSS, const platform::Render::GraphicsPipelineStateInitializer& initializer);

KeyGraphicsPipelineStateDesc GetKeyGraphicsPipelineStateDesc(
	const platform::Render::GraphicsPipelineStateInitializer& initializer, RootSignature* RootSignature);

GraphicsPipelineState::GraphicsPipelineState(const platform::Render::GraphicsPipelineStateInitializer& initializer)
	:PipelineStateInitializer(initializer)
{
	QuantizedBoundShaderState QuantizedBoundShaderState;
	QuantizeBoundShaderState(QuantizedBoundShaderState, initializer);
	
	QuantizedBoundShaderState.RootSignatureType = RootSignatureType::Raster;
	//retrive RootSignature
	RootSignature = CreateRootSignature(QuantizedBoundShaderState);

	Key =  GetKeyGraphicsPipelineStateDesc(initializer, RootSignature);

	Key.Desc.NodeMask = 0;

	Create(GraphicsPipelineStateCreateArgs(&Key, nullptr));

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


void platform_ex::Windows::D3D12::GraphicsPipelineState::Create(const GraphicsPipelineStateCreateArgs& InCreationArgs)
{
	auto Desc = InCreationArgs.Desc->Desc.GraphicsDesc();

	CheckHResult(GetDevice().GetDevice()->CreateGraphicsPipelineState(&Desc, COMPtr_RefParam(PipelineState, IID_ID3D12PipelineState)));
}

static void TranslateRenderTargetFormats(
	const platform::Render::GraphicsPipelineStateInitializer& PsoInit,
	D3D12_RT_FORMAT_ARRAY& RTFormatArray,
	DXGI_FORMAT& DSVFormat
);

KeyGraphicsPipelineStateDesc GetKeyGraphicsPipelineStateDesc(
	const platform::Render::GraphicsPipelineStateInitializer& initializer, RootSignature* RootSignature)
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
	Desc.Desc.L##S << initializer.ShaderPass.R##Shader;

	COPY_SHADER(V, Vertex);
	COPY_SHADER(P, Pixel);
	COPY_SHADER(D, Domain);
	COPY_SHADER(H, Hull);
	COPY_SHADER(G,Geometry)
#undef COPY_SHADER

	//don't support stream output

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

void QuantizeBoundShaderState(QuantizedBoundShaderState& QBSS, const platform::Render::GraphicsPipelineStateInitializer& initializer)
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
