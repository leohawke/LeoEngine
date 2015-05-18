#include "NormalUti.hlsli"

Texture2D<float> texDepth :register(t0);
Texture2D<float4> texNormalAlpha:register(t1);

SamplerState samPoint : register(s0);

cbuffer LightParam:register(b0) {
	float4 Diffuse;
	float3 Point;
}

struct VertexOut {
	float3 ViewDir :POSITION;
	float4 PosH: SV_POSITION;
	float2 Tex :TEXCOORD;
};



float4 main(VertexOut pin) : SV_TARGET
{
	float4 NormalAlpha = texNormalAlpha.Sample(samPoint, pin.Tex);
	float3 v = normalize(pin.ViewDir);
	float3 p = v*texDepth.Sample(samPoint,pin.Tex).r / v.z;
	float3 l = normalize(Point - p);
	float3 h = (v + l) / 2;
	float3 n = decode(decode(half3(NormalAlpha.rgb)));

	float lambert =saturate(dot(n, l));

	float spec = pow(max(dot(n, h),0), NormalAlpha.a*256.f);
	return float4(1.0f, 1.0f, 1.0f,spec)*Diffuse*lambert;
}