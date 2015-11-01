//filtering = min_mag_mip_point
//address_u = clamp
//address_v = clamp
SamplerState point_sampler : register(s0);

Texture2D src_tex:register(t0);


float4 CopyPS(float2 tc0 : TEXCOORD0) : SV_Target
{
	return src_tex.Sample(point_sampler, tc0);
}