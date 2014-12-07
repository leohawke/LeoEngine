#ifdef DEBUG
cbuffer LodColor
{
	float4 gColor;
};
#endif

struct PixelIn
{
	float4 PosH : SV_POSITION;
	float2 Tex : TEXCOORD;
};

Texture2D<float4> gAlphaTexture : register(t0);
Texture2DArray gMatTexture :  register(t1);
SamplerState RepeatLinear:register(s0)
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = Wrap;
	AddressV = Wrap;
};

float4 main(PixelIn pin) : SV_TARGET
{
	float4 weight = gAlphaTexture.Sample(RepeatLinear, pin.Tex);
	float4 color = gMatTexture.Sample(RepeatLinear, float3(pin.Tex, 0.f));
	float4 c0 = gMatTexture.Sample(RepeatLinear, float3(pin.Tex, 1.f));
	float4 c1 = gMatTexture.Sample(RepeatLinear, float3(pin.Tex, 2.f));
	float4 c2 = gMatTexture.Sample(RepeatLinear, float3(pin.Tex, 3.f));
	float4 c3 = gMatTexture.Sample(RepeatLinear, float3(pin.Tex, 4.f));

	color = lerp(color,c0,weight.r);
	color = lerp(color, c1, weight.g);
	color = lerp(color, c2, weight.b);
	color = lerp(color, c3, weight.a);
#ifdef DEBUG
	return color*0.75f + gColor*0.25f;
#endif
	return color;
}