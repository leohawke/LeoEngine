#include "PostProcessToneMap.h"
#include "Engine/Render/BuiltinShader.h"
#include "Engine/Render/ICommandList.h"
#include "Engine/Render/PipelineStateUtility.h"
#include "Engine/Renderer/VolumeRendering.h"
#include "Engine/Render/PipleState.h"
#include "Engine/Render/ShaderTextureTraits.hpp"

using namespace  platform::Render;

class TonemapVS : public BuiltInShader
{
public:
	EXPORTED_BUILTIN_SHADER(TonemapVS);
};

IMPLEMENT_BUILTIN_SHADER(TonemapVS, "PostProcess/PostProcessToneMap.lsl", "MainVS", platform::Render::VertexShader);

BEGIN_SHADER_PARAMETER_STRUCT(TonemapParameters)
	SHADER_PARAMETER_TEXTURE(Texture2D, ColorTexture)
	SHADER_PARAMETER_TEXTURE(Texture3D, ColorGradingLUT)
	SHADER_PARAMETER_SAMPLER(TextureSampleDesc, ColorSampler)
	SHADER_PARAMETER_SAMPLER(TextureSampleDesc, ColorGradingLUTSampler)
END_SHADER_PARAMETER_STRUCT()

class TonemapPS : public BuiltInShader
{
public:
	using Parameters = TonemapParameters;
	EXPORTED_BUILTIN_SHADER(TonemapPS);
};

IMPLEMENT_BUILTIN_SHADER(TonemapPS, "PostProcess/PostProcessToneMap.lsl", "MainPS", platform::Render::PixelShader);

void platform::TonemapPass(const TonemapInputs& Inputs)
{
	auto& CmdList = Render::GetCommandList();

	Render::RenderPassInfo passInfo(Inputs.OverrideOutput.Texture, Render::RenderTargetActions::Clear_Store);

	CmdList.BeginRenderPass(passInfo, "TonemapPass");

	Render::GraphicsPipelineStateInitializer GraphicsPSOInit {};
	CmdList.FillRenderTargetsInfo(GraphicsPSOInit);

	GraphicsPSOInit.BlendState = {};
	GraphicsPSOInit.RasterizerState.cull = Render::CullMode::None;
	GraphicsPSOInit.DepthStencilState.depth_enable = false;
	GraphicsPSOInit.DepthStencilState.depth_func = Render::CompareOp::Pass;

	auto VertexShader = Render::GetGlobalShaderMap()->GetShader<TonemapVS>();

	auto PixelShader = Render::GetGlobalShaderMap()->GetShader<TonemapPS>();

	GraphicsPSOInit.Primitive = Render::PrimtivteType::TriangleStrip;
	GraphicsPSOInit.ShaderPass.VertexDeclaration = GScreenVertexDeclaration();
	GraphicsPSOInit.ShaderPass.VertexShader = VertexShader->GetVertexShader();
	GraphicsPSOInit.ShaderPass.PixelShader = PixelShader->GetPixelShader();

	SetGraphicsPipelineState(CmdList, GraphicsPSOInit);

	TonemapParameters CommonParameters;

	SetShaderParameters(CmdList, PixelShader, PixelShader->GetPixelShader(), CommonParameters);

	//DrawFullscreenTriangle
}
