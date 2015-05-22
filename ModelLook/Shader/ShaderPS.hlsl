#include "NormalUti.hlsli"
Texture2D<float4> texNormalAlpha:register(t0);
Texture2D<float4> texLighting:register(t1);
Texture2D<float4> texDiffuseSpec:register(t2);
//Texture2D<half> texAmbient:register(t3);

SamplerState samPoint : register(s0);

struct VertexOut
{
	float4 PosH : SV_POSITION;
	float3 ViewDir :TEXCOORD0;
	float2 Tex : TEXCOORD1;
};

float3 fresnel_term_schlick(float3 light_vec, float3 halfway_vec, float3 c_spec)
{
	float e_n = saturate(dot(light_vec, halfway_vec));
	return c_spec > 0 ? c_spec + (1 - c_spec) * exp2(-(5.55473f * e_n + 6.98316f) * e_n) : 0;
}

float specular_normalize_factor(float roughness)
{
	return (roughness + 2) / 8;
}

float3 Shading(float3 diff_lighting, float3 spec_lighting, float shininess,
	float3 diffuse, float3 specular, float3 view_dir, float3 normal)
{
	return float3(max(diff_lighting * diffuse
		+ specular_normalize_factor(shininess) * spec_lighting
		//用view和normal来代替light和halfway
		* fresnel_term_schlick(normalize(view_dir), normal, specular), 0));
}

float3 Shading(float4 lighting, float shininess, float3 diffuse, float3 specular,
	float3 view_dir, float3 normal)
{
	const float3 RGB_TO_LUM = float3(0.2126f, 0.7152f, 0.0722f);
	float3 diff_lighting = lighting.rgb;
	float3 spec_lighting = lighting.a / (dot(lighting.rgb, RGB_TO_LUM) + 1e-6f) * lighting.rgb;
	return Shading(diff_lighting, spec_lighting, shininess, diffuse, specular,
		view_dir, normal);
}

float4 main(VertexOut pin) : SV_TARGET
{
	float4 NormalAlpha = texNormalAlpha.Sample(samPoint, pin.Tex);
	float3 v = pin.ViewDir;
	float3 n = decode(decode(half3(NormalAlpha.rgb)));
	float4 light = texLighting.Sample(samPoint,pin.Tex);
	float4 diffuse_spec = texDiffuseSpec.Sample(samPoint, pin.Tex);
	half ao = 1.f; //texAmbient.Sample(samPoint, pin.Tex);
	return float4(Shading(light, NormalAlpha.a*256, diffuse_spec.xyz, diffuse_spec.w,v,n)*ao,1);
}