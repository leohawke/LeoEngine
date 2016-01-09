#include <utility>
#include <NormalUti>

Texture2D texDiffuse:register(t0);
Texture2D texNormal:register(t1);

SamplerState aniso_sampler:register(s0);
SamplerState bilinear_sampler:register(s1);

struct Material
{
	float3 Specular; // w = SpecPower
	float SpecularPow;
};

cbuffer ObjectMaterial :register(b0) {
	Material Object;
}

cbuffer Camera : register(b1) {
	matrix View;
}

struct PixelIn
{
	float3 NormalW  : NORMAL;
	float4 TangentW : TANGENT;
	float2 Tex      : TEXCOORD;
	float4 PosH     : SV_POSITION;
};

void GBufferMRTPS(PixelIn pin, 
	out half4 NormalSpecPow: SV_TARGET0,
	out half4 DiffuseSpec : SV_TARGET1)
{

	float3 normalMapSample = texNormal.Sample(bilinear_sampler, pin.Tex).rgb;
	float3 bumpedNormalW = NormalSampleCalc(normalMapSample, normalize(pin.NormalW), pin.TangentW);

	NormalSpecPow.xyz = CompressionNormal(mul(bumpedNormalW,(float3x3)View));
	NormalSpecPow.w = Object.SpecularPow / 256.f;

	DiffuseSpec.rgb = texDiffuse.Sample(aniso_sampler, pin.Tex).rgb;
	DiffuseSpec.a = dot(Object.Specular, half3(0.2126f, 0.7152f, 0.0722f));
}