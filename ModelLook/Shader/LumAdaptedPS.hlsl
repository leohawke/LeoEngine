#include <utility>
Texture2D src_tex :register(t0);
//filtering =min_mag_linear_mip_point
SamplerState src_sampler : register(s0);

Texture2D last_lum_tex :register(t1);
//filtering =min_mag_mip_point
SamplerState last_lum_sampler : register(s1);

cbuffer params {
	float frame_delta;
}

float CalcAdaptedLum(float adapted_lum, float current_lum)
{
	return adapted_lum + (current_lum - adapted_lum) * (1 - pow(0.98f, 50 * frame_delta));
}

float4 LumAdapted(
	float4 iTex0 : TEXCOORD0,
	float4 iTex1 : TEXCOORD1
	) : SV_TARGET
{
	float adapted_lum = ReadAFloat(last_lum_tex.Sample(last_lum_sampler, 0.5f.xx), 16);
	float current_lum = exp(ReadAFloat(src_tex.Sample(src_sampler, 0.5f.xx), 16, -8));

	return WriteAFloat(CalcAdaptedLum(adapted_lum, current_lum), 1.0f / 16);
}