//filtering = min_mag_linear_mip_point
//address_u = clamp
//address_v = clamp
SamplerState bilinear_sampler : register(s0);

Texture2D glow_tex_0 : register(t0);
Texture2D glow_tex_1 : register(t1);
Texture2D glow_tex_2 : register(t2);

float4 GlowMergerPS(float2 tex : TEXCOORD0) : SV_Target
{
	float4 clr0 = glow_tex_0.Sample(bilinear_sampler, tex);
	float4 clr1 = glow_tex_1.Sample(bilinear_sampler, tex);
	float4 clr2 = glow_tex_2.Sample(bilinear_sampler, tex);

	return clr0 * 2.0f + clr1 * 1.15f + clr2 * 0.45f;
}