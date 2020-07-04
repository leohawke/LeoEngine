#include "PostProcessToneMap.h"
#include "Engine/Render/BuiltinShader.h"
#include "Engine/Render/ICommandList.h"
#include "Engine/Render/PipelineStateUtility.h"
#include "Engine/Render/PixelShaderUtils.h"
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

	auto PixelShader = Render::GetGlobalShaderMap()->GetShader<TonemapPS>();

	Render::RenderPassInfo passInfo(Inputs.OverrideOutput.Texture, Render::RenderTargetActions::Clear_Store);

	CmdList.BeginRenderPass(passInfo, "TonemapPass");

	Render::GraphicsPipelineStateInitializer GraphicsPSOInit {};
	PixelShaderUtils::InitFullscreenPipelineState(CmdList,*PixelShader,GraphicsPSOInit);

	auto VertexShader = Render::GetGlobalShaderMap()->GetShader<TonemapVS>();
	GraphicsPSOInit.ShaderPass.VertexShader = VertexShader->GetVertexShader();

	SetGraphicsPipelineState(CmdList, GraphicsPSOInit);

	TonemapParameters CommonParameters;
	CommonParameters.ColorGradingLUT =static_cast<Texture3D*>(Inputs.ColorGradingTexture.get());
	CommonParameters.ColorTexture = static_cast<Texture2D*>(Inputs.SceneColor.get());

	Render::TextureSampleDesc BilinearClampSampler{};
	BilinearClampSampler.address_mode_u = BilinearClampSampler.address_mode_v = BilinearClampSampler.address_mode_w = Render::TexAddressingMode::Clamp;
	BilinearClampSampler.filtering = Render::TexFilterOp::Min_Mag_Linear_Mip_Point;

	CommonParameters.ColorGradingLUTSampler = BilinearClampSampler;
	CommonParameters.ColorSampler = BilinearClampSampler;

	SetShaderParameters(CmdList, PixelShader, PixelShader->GetPixelShader(), CommonParameters);

	PixelShaderUtils::DrawFullscreenTriangle(CmdList);
}
