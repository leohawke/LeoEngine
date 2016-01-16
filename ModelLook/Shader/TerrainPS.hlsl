#include <NormalUti>
cbuffer cbParam
{
#ifdef DEBUG
	float4 gColor;
#endif
	float2 dxdy;
	float2 ex;
};

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

Texture2D<float> gHeightMap : register(t2);

SamplerState ClampPoint:register(s1);

half3 CalcNormal(float2 uv) {
	float4 lrbt = gHeightMap.GatherRed(ClampPoint,uv,int2(-1,0),int2(1,0),int2(0,1),int2(0,-1));
	float3 tangent = normalize(float3(2.f*dxdy.x,lrbt.y-lrbt.x,0.f));
	float3 bitan = normalize(float3(0.f, lrbt.z - lrbt.w, -2.f*dxdy.y));
	return normalize(cross(tangent, bitan));
}

void main(PixelIn pin,
	out half4 NormalSpecPow: SV_TARGET0,
	out half4 DiffuseSpec : SV_TARGET1)
{
	half3 normal = CalcNormal(pin.Tex);
	NormalSpecPow.xyz = CompressionNormal(normal);
	NormalSpecPow.w = 1.f / 256.f;

#ifdef DEBUG
	DiffuseSpec.xyz = gColor.xyz;
#else
	float4 weight = gAlphaTexture.Sample(RepeatLinear, pin.Tex);
	float4 color = gMatTexture.Sample(RepeatLinear, float3(pin.Tex, 0.f));
	float4 c0 = gMatTexture.Sample(RepeatLinear, float3(pin.Tex, 1.f));
	float4 c1 = gMatTexture.Sample(RepeatLinear, float3(pin.Tex, 2.f));
	float4 c2 = gMatTexture.Sample(RepeatLinear, float3(pin.Tex, 3.f));
	float4 c3 = gMatTexture.Sample(RepeatLinear, float3(pin.Tex, 4.f));
	color = lerp(color, c0, weight.r);
	color = lerp(color, c1, weight.g);
	color = lerp(color, c2, weight.b);
	color = lerp(color, c3, weight.a);
	DiffuseSpec.xyz = color.xyz;
#endif
	DiffuseSpec.w = 0.1f;
}