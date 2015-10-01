Texture2D original:register(t0);//original
Texture2D star:register(t1);//star
Texture2D bloom:register(t2);//bloom

SamplerState s0 : register(s0);//POINT
SamplerState s1 : register(s1);//LINEAR


// The per-color weighting to be used for luminance calculations in RGB order.
static const float3 LUMINANCE_VECTOR = float3(0.2125f, 0.7154f, 0.0721f);

// The per-color weighting to be used for blue shift under low light.
static const float3 BLUE_SHIFT_VECTOR = float3(1.05f, 0.97f, 1.27f);

cbuffer Params :register(c0) {
	float fAdaptedLum;
	float g_fMiddleGray;//default=0.18f
	float g_fStarScale;//default=0.5f
	float g_fBloomScale;//default=1.f
}

float4 HDRFinal
(
	in float4 PosH:SV_POSITION,
	in float2 Tex : TEXCOORD
	) : SV_TARGET
{
	float4 vSample = original.Sample(s0, Tex);
	float4 vBloom = bloom.Sample(s1,Tex);
	float4 vStar = star.Sample(s1, Tex);

	// For very low light conditions, the rods will dominate the perception
	// of light, and therefore color will be desaturated and shifted
	// towards blue.
	// Define a linear blending from -1.5 to 2.6 (log scale) which
	// determines the lerp amount for blue shift
	float fBlueShiftCoefficient = 1.0f - (fAdaptedLum + 1.5) / 4.1;
	fBlueShiftCoefficient = saturate(fBlueShiftCoefficient);

	// Lerp between current color and blue, desaturated copy
	float3 vRodColor = dot((float3)vSample, LUMINANCE_VECTOR) * BLUE_SHIFT_VECTOR;
	vSample.rgb = lerp((float3)vSample, vRodColor, fBlueShiftCoefficient);

	// Map the high range of color values into a range appropriate for
	// display, taking into account the user's adaptation level, and selected
	// values for for middle gray and white cutoff.
	vSample.rgb *= g_fMiddleGray / (fAdaptedLum + 0.001f);
	vSample.rgb /= (1.0f + vSample.rgb);

	// Add the star and bloom post processing effects
	vSample += g_fStarScale * vStar;
	vSample += g_fBloomScale * vBloom;

	return vSample;
}