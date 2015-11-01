//filtering = min_mag_linear_mip_point
//address_u = clamp
//address_v = clamp
SamplerState bilinear_sampler : register(s0);

Texture2D src_tex:register(t0);

float4 BilinearCopyPS(float2 tc0 : TEXCOORD0) : SV_Target
{
	return src_tex.Sample(bilinear_sampler, tc0);
}