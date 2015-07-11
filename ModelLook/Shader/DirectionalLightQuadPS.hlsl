#include "NormalUti.hlsli"
#include "Lighting.hlsli"
Texture2D<float> texDepth :register(t0);
Texture2D<float4> texNormalAlpha:register(t1);

SamplerState samPoint : register(s0);

struct DirectionalLight {
	float3 Directional;
	float3 Diffuse;
};

cbuffer LightParam:register(b0) {
	DirectionalLight Light;
}

struct VertexOut
{
	float4 PosH : SV_POSITION;
	float3 ViewDir :TEXCOORD0; 
	float2 Tex : TEXCOORD1;
};


float4 main(VertexOut pin) : SV_Target
{
	float2 tc = pin.Tex;
	float4 NormalAlpha = texNormalAlpha.Sample(samPoint, tc);
	float3 view_dir = normalize(pin.ViewDir);
	float3 normal = DeCompressionNormal(half3(NormalAlpha.rgb));
	float3 dir = Light.Directional;
	float3 light_color = Light.Diffuse;
	float n_dot_l = dot(normal, dir);
	float4 lighting = 0;
	if (n_dot_l > 0)
	{
		float spec = roughness_term(normalize(dir - view_dir), normal, NormalAlpha.w*256.f).x;
		lighting = CalcColor(n_dot_l, spec, 1, light_color);
	}

	return lighting;
}