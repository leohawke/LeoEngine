Texture2D src_tex:register(t0);
Texture2D bloom_tex:register(t1);
Texture2D lum_tex:register(t2);

//filtering = min_mag_linear_mip_point
//address_u = clamp
//address_v = clamp
SamplerState point_sampler : register(s0);

//filtering = min_mag_mip_point
//address_u = clamp
//address_v = clamp
SamplerState linear_sampler : register(s1);

// The per-color weighting to be used for luminance calculations in RGB order.
static const float3 RGB_TO_LUM = float3(0.2126f, 0.7152f, 0.0722f);


cbuffer Params :register(c0) {
	float fAdaptedLum;
	float g_fMiddleGray;//default=1.f
	float g_fBloomScale;//default=0.25f
}

float EyeAdaption(float lum){
	return lerp(0.2f, lum, 0.5f);
}

float3 F(float3 x) {
	const float A = 0.22f;
	const float B = 0.30f;
	const float C = 0.10f;
	const float D = 0.20f;
	const float E = 0.01f;
	const float F = 0.30f;

	return ((x * (A * x + C * B) + D * E) / (x * (A * x + B) + D * F)) - E / F;
}

float3 ToneMapping(float3 color, float3 blur, float adapted_lum) {
	const float3 BLUE_SHIFT = float3(0.4f, 0.4f, 0.7f);

	color += blur * g_fBloomScale;

	float lum = dot(color, RGB_TO_LUM);

	// martin's modified blue shift
	color = lerp(lum * BLUE_SHIFT, color, saturate(16.0f * lum));

	float adapted_lum_dest = 3 / (max(0.1f, 1 + 10 * EyeAdaption(adapted_lum)));

	// Filmic Tonemapping from Unchart 2
	const float White = 11.2f;
	return F(g_fMiddleGray * 1.6f * adapted_lum_dest * color) / F(White);
}

// The per-color weighting to be used for blue shift under low light.
static const float3 BLUE_SHIFT_VECTOR = float3(1.05f, 0.97f, 1.27f);

float4 HDRFinal
(
	in float4 PosH:SV_POSITION,
	in float2 Tex : TEXCOORD
	) : SV_TARGET
{
	//note FXAA use the lum result in w channel;
	float3 ldr_rgb = saturate(ToneMapping(src_tex.Sample(linear_sampler,Tex).rgb, bloom_tex.Sample(linear_sampler,Tex).rgb,fAdaptedLum));
	return float4(ldr_rgb, dot(ldr_rgb, RGB_TO_LUM));
}