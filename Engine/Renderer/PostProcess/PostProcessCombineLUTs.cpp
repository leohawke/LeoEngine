#include "PostProcessCombineLUTs.h"
#include <Engine/Render/ShaderParamterTraits.hpp>
#include <Engine/Render/BuiltInShader.h>
#include <Engine/Render/IContext.h>
#include <LBase/smart_ptr.hpp>
#include <Engine/Render/ICommandList.h>
#include "../VolumeRendering.h"

using namespace platform;
using namespace platform::Render::Shader;

platform::ColorCorrectParameters::ColorCorrectParameters()
{
	ColorSaturation = leo::math::float4(1.0f, 1.0f, 1.0f, 1.0f);
	ColorContrast = leo::math::float4(1.0f, 1.0f, 1.0f, 1.0f);
	ColorGamma = leo::math::float4(1.0f, 1.0f, 1.0f, 1.0f);
	ColorGain = leo::math::float4(1.0f, 1.0f, 1.0f, 1.0f);
	ColorOffset = leo::math::float4(0.0f, 0.0f, 0.0f, 0.0f);

	ColorSaturationShadows = leo::math::float4(1.0f, 1.0f, 1.0f, 1.0f);
	ColorContrastShadows = leo::math::float4(1.0f, 1.0f, 1.0f, 1.0f);
	ColorGammaShadows = leo::math::float4(1.0f, 1.0f, 1.0f, 1.0f);
	ColorGainShadows = leo::math::float4(1.0f, 1.0f, 1.0f, 1.0f);
	ColorOffsetShadows = leo::math::float4(0.0f, 0.0f, 0.0f, 0.0f);

	ColorSaturationMidtones = leo::math::float4(1.0f, 1.0f, 1.0f, 1.0f);
	ColorContrastMidtones = leo::math::float4(1.0f, 1.0f, 1.0f, 1.0f);
	ColorGammaMidtones = leo::math::float4(1.0f, 1.0f, 1.0f, 1.0f);
	ColorGainMidtones = leo::math::float4(1.0f, 1.0f, 1.0f, 1.0f);
	ColorOffsetMidtones = leo::math::float4(0.f, 0.0f, 0.0f, 0.0f);

	ColorSaturationHighlights = leo::math::float4(1.0f, 1.0f, 1.0f, 1.0f);
	ColorContrastHighlights = leo::math::float4(1.0f, 1.0f, 1.0f, 1.0f);
	ColorGammaHighlights = leo::math::float4(1.0f, 1.0f, 1.0f, 1.0f);
	ColorGainHighlights = leo::math::float4(1.0f, 1.0f, 1.0f, 1.0f);
	ColorOffsetHighlights = leo::math::float4(0.0f, 0.0f, 0.0f, 0.0f);
}

class LUTBlenderShader : public BuiltInShader
{
	//SetDefine("USE_VOLUME_LUT", true);
};

BEGIN_SHADER_PARAMETER_STRUCT(CombineLUTParameters)
SHADER_PARAMETER(leo::math::float4, ColorSaturation)
SHADER_PARAMETER(leo::math::float4, ColorContrast)
SHADER_PARAMETER(leo::math::float4, ColorGamma)
SHADER_PARAMETER(leo::math::float4, ColorGain)
SHADER_PARAMETER(leo::math::float4, ColorOffset)
SHADER_PARAMETER(leo::math::float4, ColorSaturationShadows)
SHADER_PARAMETER(leo::math::float4, ColorContrastShadows)
SHADER_PARAMETER(leo::math::float4, ColorGammaShadows)
SHADER_PARAMETER(leo::math::float4, ColorGainShadows)
SHADER_PARAMETER(leo::math::float4, ColorOffsetShadows)
SHADER_PARAMETER(leo::math::float4, ColorSaturationMidtones)
SHADER_PARAMETER(leo::math::float4, ColorContrastMidtones)
SHADER_PARAMETER(leo::math::float4, ColorGammaMidtones)
SHADER_PARAMETER(leo::math::float4, ColorGainMidtones)
SHADER_PARAMETER(leo::math::float4, ColorOffsetMidtones)
SHADER_PARAMETER(leo::math::float4, ColorSaturationHighlights)
SHADER_PARAMETER(leo::math::float4, ColorContrastHighlights)
SHADER_PARAMETER(leo::math::float4, ColorGammaHighlights)
SHADER_PARAMETER(leo::math::float4, ColorGainHighlights)
SHADER_PARAMETER(leo::math::float4, ColorOffsetHighlights)

SHADER_PARAMETER(float,ColorCorrectionShadowsMax)
SHADER_PARAMETER(float,ColorCorrectionHighlightsMin)
END_SHADER_PARAMETER_STRUCT();


class LUTBlenderPS : public LUTBlenderShader
{
public:
	EXPORTED_SHADER_TYPE(LUTBlenderPS);
};

IMPLEMENT_SHADER(LUTBlenderPS, "PostProcessCombineLUTs.lsl", "MainPS", platform::Render::PixelShader);

void GetCombineLUTParameters(CombineLUTParameters& Parameters, const ColorCorrectParameters& args)
{
#define COPY(Member) Parameters.Member = args.Member
	COPY(ColorSaturation);
	COPY(ColorContrast);
	COPY(ColorGamma);
	COPY(ColorGain);
	COPY(ColorOffset);
	COPY(ColorSaturationShadows);
	COPY(ColorContrastShadows);
	COPY(ColorGammaShadows);
	COPY(ColorGainShadows);
	COPY(ColorOffsetShadows);
	COPY(ColorSaturationMidtones);
	COPY(ColorContrastMidtones);
	COPY(ColorGammaMidtones);
	COPY(ColorGainMidtones);
	COPY(ColorOffsetMidtones);
	COPY(ColorSaturationHighlights);
	COPY(ColorContrastHighlights);
	COPY(ColorGammaHighlights);
	COPY(ColorGainHighlights);
	COPY(ColorOffsetHighlights);
#undef COPY
}

constexpr int32 GLUTSize = 32;

std::shared_ptr<Render::Texture> platform::CombineLUTPass(const ColorCorrectParameters& args)
{
	const bool bUseVolumeTextureLUT = true;

	Render::Texture* OutputTexture = nullptr;

	if(bUseVolumeTextureLUT)
		OutputTexture = Render::Context::Instance().GetDevice().CreateTexture(GLUTSize, GLUTSize, GLUTSize, 1, 1, Render::EF_BGR32F, Render::EA_GPURead | Render::EA_GPUWrite, { 1,0 });

	Render::RenderTargetView* pRT = nullptr; // Render::Context::Instance().GetDevice().CreateRenderTargetView(OutputTexture);

	auto& CmdList = Render::GetCommandList();

	Render::RenderPassInfo passInfo;
	passInfo.ColorRenderTargets[0] = pRT;
	passInfo.DepthStencilTarget = nullptr;

	CmdList.BeginRenderPass(passInfo,"CombineLUTPass");

	Render::GraphicsPipelineStateInitializer GraphicsPSOInit;
	CmdList.FillRenderTargetsInfo(GraphicsPSOInit);

	GraphicsPSOInit.BlendState = {};
	GraphicsPSOInit.RasterizerState = {};
	GraphicsPSOInit.DepthStencilState.depth_enable = false;
	GraphicsPSOInit.DepthStencilState.depth_func = Render::CompareOp::Pass;

	CombineLUTParameters PassParameters;
	GetCombineLUTParameters(PassParameters,args);

	if (bUseVolumeTextureLUT)
	{
		const VolumeBounds Bounds(GLUTSize);

		auto VertexShader = Render::GetGlobalShaderMap()->GetShader<WriteToSliceVS>();

		auto GeometryShader = Render::GetGlobalShaderMap()->GetShader<WriteToSliceGS>();

		auto PixelShader = Render::GetGlobalShaderMap()->GetShader<LUTBlenderPS>();

		GraphicsPSOInit.Primitive = Render::PrimtivteType::TriangleStrip;
		GraphicsPSOInit.ShaderState = nullptr;//CreateRenderPass(VertexShader,GeometryShader,PixelShader)

		//SetGraphicsPipelineState(CmdList, GraphicsPSOInit);

		VertexShader->SetParameters(CmdList, Bounds, leo::math::int3(Bounds.MaxX - Bounds.MinX));

		//SetShaderParameters(CmdList, *PixelShader, PixelShader->GetPixelShader(), *PassParameters);

		RasterizeToVolumeTexture(CmdList, Bounds);
	}

	return leo::share_raw(OutputTexture);
}
