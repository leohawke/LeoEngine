#include "NormalUti.hlsli"
#include "Lighting.hlsli"
Texture2D<float> texDepth :register(t0);
Texture2D<float4> texNormalAlpha:register(t1);

SamplerState samPoint : register(s0);

struct SpotLight {
	float4 Position_Inner;
	float3 Diffuse;
	float4 FallOff_Range;
	float4 Directional_Outer;
};

cbuffer LightParam:register(b0) {
	SpotLight Light;
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
	float3 normal = DeCompressionNormal(half3(NormalAlpha.rgb));

	float spot = spot_lighting(Light.Position_Inner.xyz, Light.Directional_Outer.xyz,
		float2(Light.Directional_Outer.w, Light.Position_Inner.w), p);

	return CalcDRLighting(Light.Position_Inner.xyz, p, normal, v,
		NormalAlpha.w*256.f,
		spot*attenuation_term(Light.Position_Inner.xyz, p, Light.FallOff_Range.xyz),
		Light.Diffuse,
		Light.FallOff_Range.w
		);
}