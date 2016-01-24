#include <NormalUti>
#include <Lighting>
#include <utility>
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


cbuffer SkylightParam :register(b0)
{
	float4x4 inv_view;
	int3 skylight_diff_spec_mip;
	float skylight_mip_bias;
};

TextureCube skylight_y_cube_tex:register(t3);
TextureCube skylight_c_cube_tex:register(t4);
SamplerState skylight_sampler : register(s1);

float4 SkylightShading(float shininess, float4 mrt1, float3 normal, float3 view)
{
	float4 shading = 0;

	if (skylight_diff_spec_mip.z)
	{
		float3 c_diff = GetDiffuse(mrt1);
		float c_spec = GetSpecular(mrt1);

		normal = mul(normal, (float3x3)inv_view);
		view = mul(view, (float3x3)inv_view);

		float3 prefiltered_clr = decode_hdr_yc(TexCubeSampleLevel(skylight_y_cube_tex, skylight_sampler, normal, skylight_diff_spec_mip.x, 0).r,
			TexCubeSampleLevel(skylight_c_cube_tex, skylight_sampler, normal, skylight_diff_spec_mip.x, 0)).xyz;
		shading.xyz += CalcEnvDiffuse(prefiltered_clr, c_diff);

		shininess = log2(shininess) / 13; // log2(8192) == 13

		float mip = CalcPrefilteredEnvMip(shininess, skylight_diff_spec_mip.y);
		float3 r = CalcPrefilteredEnvVec(normal, view);
		prefiltered_clr = decode_hdr_yc(TexCubeSampleLevel(skylight_y_cube_tex, skylight_sampler, r, mip, skylight_mip_bias).r,
			TexCubeSampleLevel(skylight_c_cube_tex, skylight_sampler, r, mip, skylight_mip_bias)).xyz;
		shading.xyz += CalcEnvSpecular(prefiltered_clr, c_spec, shininess, normal, view);
	}

	return shading;
}

float4 main(VertexOut pin) : SV_TARGET
{
	float4 NormalAlpha = texNormalAlpha.Sample(samPoint, pin.Tex);
	float3 v = pin.ViewDir;
	float3 n = DeCompressionNormal(half3(NormalAlpha.rgb));
	float4 light = texLighting.Sample(samPoint,pin.Tex);
	float4 diffuse_spec = texDiffuseSpec.Sample(samPoint, pin.Tex);
	half ao = 1.f; //texAmbient.Sample(samPoint, pin.Tex);
	return float4(Shading(light, NormalAlpha.a*256, diffuse_spec.xyz, diffuse_spec.w,v,n)*ao,1);
}


float4 skymain(VertexOut pin) : SV_TARGET
{
	float4 NormalAlpha = texNormalAlpha.Sample(samPoint, pin.Tex);
	float shininess = NormalAlpha.a * 256;
	float3 normal = DeCompressionNormal(half3(NormalAlpha.rgb));
	float3 view_dir = pin.ViewDir;
	float4 mrt1 = texDiffuseSpec.Sample(samPoint, pin.Tex);

	return SkylightShading(shininess, mrt1, normal, view_dir);
}