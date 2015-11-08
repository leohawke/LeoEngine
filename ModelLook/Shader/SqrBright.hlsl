//filtering = min_mag_linear_mip_point
//address_u = clamp
//address_v = clamp
SamplerState bilinear_sampler : register(s0);

Texture2D src_tex:register(t0);

float4 SqrBrightPS(float2 tc : TEXCOORD0) : SV_Target
{
	float4 clr = src_tex.Sample(bilinear_sampler, tc);
	return clamp(clr * (clr / 3), 0, 32);
}