#include "PostProcessCombineLUTs.h"

using namespace platform;


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

leo::shared_ptr<Render::Texture> platform::CombineLUTPass()
{
	return leo::shared_ptr<Render::Texture>();
}
