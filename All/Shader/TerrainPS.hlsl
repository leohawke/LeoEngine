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

Texture2D<float3> gAlphaTexture : register(t0);
Texture2DArray gMatTexture :  register(t1);
SamplerState RepeatLinear:register(s0)
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = Wrap;
	AddressV = Wrap;
};

float4 main(PixelIn pin) : SV_TARGET
{
	float3 weight = gAlphaTexture.Sample(RepeatLinear, pin.Tex).xyz;
	weight = normalize(weight);
		float4 color = weight.x *gMatTexture.Sample(RepeatLinear, float3(pin.Tex, 0.f));
		color += weight.y*gMatTexture.Sample(RepeatLinear, float3(pin.Tex, 1.f));
	color += weight.z*gMatTexture.Sample(RepeatLinear, float3(pin.Tex, 2.f));
	return color;
}