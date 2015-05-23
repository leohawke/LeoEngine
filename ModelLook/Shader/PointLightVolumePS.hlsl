#include "NormalUti.hlsli"
#include "Lighting.hlsli"
Texture2D<float> texDepth :register(t0);
Texture2D<float4> texNormalAlpha:register(t1);

SamplerState samPoint : register(s0);

struct PointLight {
	float3 Position;
	float3 Diffuse;
	float4 FallOff_Range;
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
	float3 normal = decode(decode(half3(NormalAlpha.rgb)));

	return CalcDRLighting(Light.Position, p, normal, v,
		NormalAlpha.w*256.f,
		attenuation_term(Light.Position, p, Light.FallOff_Range.xyz),
		Light.Diffuse,
		Light.FallOff_Range.w
		);
}