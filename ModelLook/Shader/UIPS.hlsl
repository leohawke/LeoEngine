cbuffer ui_psparams : register(b0) {
	float4 color;
}

Texture2D src_tex;
SamplerState bilinear_sampler;

float4 main(float2 tc0:TEXCOORD0) : SV_TARGET
{
	return src_tex.Sample(bilinear_sampler, tc0)*color;
}