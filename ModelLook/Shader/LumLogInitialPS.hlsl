#include <utility>
Texture2D src_tex;

SamplerState src_sampler : register(s0);

// The per-color weighting to be used for luminance calculations in RGB order.
static const float3 RGB_TO_LUM = float3(0.2126f, 0.7152f, 0.0722f);

//-----------------------------------------------------------------------------                                
// Desc: Sample the luminance of the source image using a kernal of sample
//       points, and return a scaled image containing the log() of averages
//-----------------------------------------------------------------------------
float4 LumLogInitial
(
	float4 iTex0 : TEXCOORD0,
	float4 iTex1 : TEXCOORD1
	) : SV_TARGET
{
	float4 tex[2] = { iTex0, iTex1 };

	float s = 0;
	for (int i = 0; i < 2; ++i)
	{
		s += log(dot(src_tex.Sample(src_sampler, tex[i].xy).rgb, RGB_TO_LUM) + 0.001f);
		s += log(dot(src_tex.Sample(src_sampler, tex[i].zw).rgb, RGB_TO_LUM) + 0.001f);
	}

	return WriteAFloat(s / 4, 1.0f / 16, 0.5f);
}