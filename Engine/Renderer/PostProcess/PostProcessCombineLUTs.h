#pragma once

#include <LBase/lmathtype.hpp>
#include "../../Render/ITexture.hpp"

namespace platform
{
	struct CombineLUTSettings
	{
		CombineLUTSettings();

		leo::math::float4 ColorSaturation;
		leo::math::float4 ColorContrast;
		leo::math::float4 ColorGamma;
		leo::math::float4 ColorGain;
		leo::math::float4 ColorOffset;
		leo::math::float4 ColorSaturationShadows;
		leo::math::float4 ColorContrastShadows;
		leo::math::float4 ColorGammaShadows;
		leo::math::float4 ColorGainShadows;
		leo::math::float4 ColorOffsetShadows;
		leo::math::float4 ColorSaturationMidtones;
		leo::math::float4 ColorContrastMidtones;
		leo::math::float4 ColorGammaMidtones;
		leo::math::float4 ColorGainMidtones;
		leo::math::float4 ColorOffsetMidtones;
		leo::math::float4 ColorSaturationHighlights;
		leo::math::float4 ColorContrastHighlights;
		leo::math::float4 ColorGammaHighlights;
		leo::math::float4 ColorGainHighlights;
		leo::math::float4 ColorOffsetHighlights;

		float ColorCorrectionShadowsMax = 0.09f;
		float ColorCorrectionHighlightsMin = 0.5f;

		float WhiteTemp;
		float WhiteTint;

		/** Correct for artifacts with "electric" blues due to the ACEScg color space. Bright blue desaturates instead of going to violet. */
		float BlueCorrection;
		/** Expand bright saturated colors outside the sRGB gamut to fake wide gamut rendering. */
		float ExpandGamut;

		float FilmSlope;
		float FilmToe;
		float FilmShoulder;
		float FilmBlackClip;
		float FilmWhiteClip;
	};

	std::shared_ptr<Render::Texture> CombineLUTPass(const CombineLUTSettings& args);
}
