#include "LightBuffer.hlsli"

iBaseLight gAbsLight;

struct PixelIn
{
	float4 PosH     : SV_POSITION;
	float4 Shadow_PosH : POSITION1;
	float3 PosW     : POSITION0;
	float3 NormalW  : NORMAL;
	float3 TangentW : TANGENT;
	float2 Tex      : TEXCOORD;
};

struct PixelOut {
	float4 Normal : SV_Target0;
	float4 Diffuse : SV_Target1;
	float4 Specular : SV_Target2;
	float4 Pos : SV_Target3;
};

SamplerState LinearRepeat :register(s0);

SamplerComparisonState samShadow : register(s1);

#if 0
float4 main(PixelIn pin) : SV_TARGET
{
	float4 texColor = TexDiffuse.Sample(LinearRepeat, pin.Tex);
	clip(texColor.a - 0.1f);

	//lerp can unnormalize
	pin.NormalW = normalize(pin.NormalW);
	float3 normalMapSample = TexNormalMap.Sample(LinearRepeat, pin.Tex).rgb;
		float3 bumpedNormalW = NormalSampleToWorldSpace(normalMapSample, pin.NormalW, pin.TangentW);

		float3 v = normalize(gEyePosW - pin.PosW);
		float shadow =1.f;
	shadow = CalcShadowFactor(samShadow, ShadowMap, pin.Shadow_PosH);
	float4 ambient = gAbsLight.Ambient(bumpedNormalW)*gMat.ambient;
		float4 diffuse = gAbsLight.Diffuse(bumpedNormalW, pin.PosW, v)*gMat.diffuse;
		float4 spec = gAbsLight.Specular(bumpedNormalW, gMat.specular.w, pin.PosW, v)*gMat.specular;


	diffuse *= shadow;
	spec *= shadow;
	ambient *= shadow;
	float4 litColor = texColor*(ambient + diffuse) + spec;
	litColor.a = texColor.a;
	return litColor;
}
#endif

PixelOut main(PixelIn pin) {
	float4 texColor = TexDiffuse.Sample(LinearRepeat, pin.Tex);
	clip(texColor.a - 0.1f);

	//lerp can unnormalize
	pin.NormalW = normalize(pin.NormalW);
	float3 normalMapSample = TexNormalMap.Sample(LinearRepeat, pin.Tex).rgb;
	float3 bumpedNormalW = NormalSampleToWorldSpace(normalMapSample, pin.NormalW, pin.TangentW);

	float shadow = 1.f;
	shadow = CalcShadowFactor(samShadow, ShadowMap, pin.Shadow_PosH);
	float4 diffuse = gMat.diffuse*shadow;
	float4 spec = float4(gMat.specular.xyz*shadow, gMat.specular.w);

	PixelOut pout;
	pout.Normal =float4(bumpedNormalW,1.0f);
	pout.Diffuse = diffuse;
	pout.Specular = spec;
	pout.Pos =float4(pin.PosW,1.0f);
	return pout;
}