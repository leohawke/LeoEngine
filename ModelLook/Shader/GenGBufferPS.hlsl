#include <NormalUti>

SamplerState samAniso:register(s0);

Texture2D texDiffuse:register(t0);

struct Material
{
	float3 Specular; // w = SpecPower
	float SpecularPow;
};

cbuffer ObjectMaterial :register(b0) {
	Material Object;
}

struct VertexOut
{
	float4 PosH     : SV_POSITION;
	half3 NormalV  : NORMAL;
	float2 Tex      : TEXCOORD;
};



void GBufferMRTPS(VertexOut pin,
	out half4 NormalSpecPow: SV_TARGET0,
	out half4 DiffuseSpec : SV_TARGET1)
{
	NormalSpecPow.xyz = CompressionNormal(pin.NormalV);
	NormalSpecPow.w = Object.SpecularPow/256.f;

	DiffuseSpec.rgb = texDiffuse.Sample(samAniso, pin.Tex).rgb;
	DiffuseSpec.a = dot(Object.Specular,half3(0.2126f, 0.7152f, 0.0722f));
}