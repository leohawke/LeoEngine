#include <utility>
Texture2D src_tex;

SamplerState src_sampler : register(s0);


float4 LumIterative(
	float4 iTex0 : TEXCOORD0,
	float4 iTex1 : TEXCOORD1
	) : SV_TARGET
{
	float4 tex[2] = { iTex0, iTex1 };

	float s = 0;
	for (int i = 0; i < 2; ++i)
	{
		s += ReadAFloat(src_tex.Sample(src_sampler, tex[i].xy), 16, -8);
		s += ReadAFloat(src_tex.Sample(src_sampler, tex[i].zw), 16, -8);
	}

	return WriteAFloat(s / 4, 1.0f / 16, 0.5f);
}