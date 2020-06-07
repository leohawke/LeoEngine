#include <LBase/ldef.h>
#include "GraphicsPipelineState.h"
#include "Convert.h"
#include "RootSignature.h"
#include "ShaderCompose.h"
#include "VertexDeclaration.h"
#include "Context.h"

using namespace platform_ex::Windows::D3D12;



KeyGraphicsPipelineStateDesc GetKeyGraphicsPipelineStateDesc(
	const platform::Render::GraphicsPipelineStateInitializer& initializer, RootSignature* RootSignature);

GraphicsPipelineState::GraphicsPipelineState(const platform::Render::GraphicsPipelineStateInitializer& initializer)
	:PipelineStateInitializer(initializer)
{
	//retrive RootSignature
	auto root_signature = static_cast<platform_ex::Windows::D3D12::RootSignature*>(nullptr);

	Key =  GetKeyGraphicsPipelineStateDesc(initializer, root_signature);

	Key.Desc.NodeMask = 0;

	Create(GraphicsPipelineStateCreateArgs(&Key, nullptr));
}


void platform_ex::Windows::D3D12::GraphicsPipelineState::Create(const GraphicsPipelineStateCreateArgs& InCreationArgs)
{
	auto Desc = InCreationArgs.Desc->Desc.GraphicsDesc();

	Context::Instance().GetDevice().GetDevice()->CreateGraphicsPipelineState(&Desc, COMPtr_RefParam(PipelineState, IID_ID3D12PipelineState));
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
		Desc.Desc.InputLayout.NumElements = VertexDeclaration->VertexElements.size();
		Desc.Desc.InputLayout.pInputElementDescs = VertexDeclaration->VertexElements.data();
	}

	//CopyShader
#define COPY_SHADER(L,R) \
	Desc.Desc.L##S << initializer.ShaderPass.R##Shader;

	//COPY_SHADER(V, Vertex);
	//COPY_SHADER(P, Pixel);
	//COPY_SHADER(D, Domain);
	//COPY_SHADER(H, Hull);
	//COPY_SHADER(G,Geometry)
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
