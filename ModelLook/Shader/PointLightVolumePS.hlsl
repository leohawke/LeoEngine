#include "NormalUti.hlsli"
#include "Lighting.hlsli"
Texture2D<float> texDepth :register(t0);
Texture2D<float4> texNormalAlpha:register(t1);

SamplerState samPoint : register(s0);

struct PointLight {
	float4 PositionRange;
	float3 Diffuse;
};

cbuffer LightParam:register(b0) {
	PointLight Light;
}

struct VertexOut {
	float3 ViewDir :POSITION;
	float4 PosH: SV_POSITION;
	float3 Tex :TEXCOORD;
};



float4 main(VertexOut pin) : SV_TARGET
{
	float2 tc = pin.Tex.xy / pin.Tex.z;
	float4 NormalAlpha = texNormalAlpha.Sample(samPoint, tc);
	float3 v = normalize(pin.ViewDir);
	float3 p = v*texDepth.Sample(samPoint, tc).r / v.z;
	float3 l = normalize(Light.PositionRange.xyz - p);
	float3 h = normalize(l-v);
	float3 n = decode(decode(half3(NormalAlpha.rgb)));

	float lambert =saturate(dot(n, l));

	float spec = roughness_term(h,n,NormalAlpha.a*256.f);
	return float4(1.0f, 1.0f, 1.0f,spec)*float4(Light.Diffuse,1.f)*lambert;
}