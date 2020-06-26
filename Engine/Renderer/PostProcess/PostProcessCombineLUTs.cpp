#include "PostProcessCombineLUTs.h"
#include <Engine/Render/ShaderParamterTraits.hpp>
#include <Engine/Render/BuiltInShader.h>
#include <Engine/Render/IContext.h>
#include <Engine/Render/PipelineStateUtility.h>
#include <Engine/Render/ShaderParameterStruct.h>
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

	WhiteTemp = 6500;
	WhiteTint = 0;

	// ACES settings
	FilmSlope = 0.88f;
	FilmToe = 0.55f;
	FilmShoulder = 0.26f;
	FilmBlackClip = 0.0f;
	FilmWhiteClip = 0.04f;

	BlueCorrection = 0.6f;
	ExpandGamut = 1.0f;
}

class LUTBlenderShader : public BuiltInShader
{
public:
	using DerivedType = LUTBlenderShader;
	using BuiltInShader::BuiltInShader;
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
SHADER_PARAMETER(float, WhiteTemp)
SHADER_PARAMETER(float, WhiteTint)
SHADER_PARAMETER(float, BlueCorrection)
SHADER_PARAMETER(float, ExpandGamut)
SHADER_PARAMETER(float, FilmSlope)
SHADER_PARAMETER(float, FilmToe)
SHADER_PARAMETER(float, FilmShoulder)
SHADER_PARAMETER(float, FilmBlackClip)
SHADER_PARAMETER(float, FilmWhiteClip)
END_SHADER_PARAMETER_STRUCT();


class LUTBlenderPS : public LUTBlenderShader
{
public:
	using Parameters = CombineLUTParameters;
	EXPORTED_BUILTIN_SHADER(LUTBlenderPS);
};

IMPLEMENT_BUILTIN_SHADER(LUTBlenderPS, "PostProcess/PostProcessCombineLUTs.lsl", "MainPS", platform::Render::PixelShader);

void GetCombineLUTParameters(CombineLUTParameters& Parameters, const ColorCorrectParameters& args)
{
	std::memcpy(&Parameters.ColorSaturation, &args.ColorSaturation, loffsetof(ColorCorrectParameters, FilmWhiteClip) - loffsetof(ColorCorrectParameters, ColorSaturation));
}

constexpr int32 GLUTSize = 32;

std::shared_ptr<Render::Texture> platform::CombineLUTPass(const ColorCorrectParameters& args)
{
	const bool bUseVolumeTextureLUT = true;

	Render::Texture* OutputTexture = nullptr;

	Render::Texture3DInitializer initializer;
	initializer.Width = initializer.Height = initializer.Depth = GLUTSize;
	initializer.ArraySize = 1;
	initializer.NumMipmaps = 1;
	initializer.Format = Render::EF_ABGR16F;
	initializer.Access = Render::EA_GPURead | Render::EA_GPUWrite;
	initializer.NumSamples = 1;
	if(bUseVolumeTextureLUT)
		OutputTexture = Render::Context::Instance().GetDevice().CreateTexture(initializer,Render::TexCreate_RenderTargetable);

	auto& CmdList = Render::GetCommandList();

	Render::RenderPassInfo passInfo(OutputTexture,Render::RenderTargetActions::Clear_Store);

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
		GraphicsPSOInit.ShaderPass.VertexDeclaration = GScreenVertexDeclaration();
		GraphicsPSOInit.ShaderPass.VertexShader = VertexShader->GetVertexShader();
		GraphicsPSOInit.ShaderPass.GeometryShader = GeometryShader->GetGeometryShader();
		GraphicsPSOInit.ShaderPass.PixelShader = PixelShader->GetPixelShader();

		SetGraphicsPipelineState(CmdList, GraphicsPSOInit);

		VertexShader->SetParameters(CmdList, Bounds, leo::math::int3(Bounds.MaxX - Bounds.MinX));

		SetShaderParameters(CmdList, PixelShader, PixelShader->GetPixelShader(), PassParameters);

		RasterizeToVolumeTexture(CmdList, Bounds);
	}

	return leo::share_raw(OutputTexture);
}
