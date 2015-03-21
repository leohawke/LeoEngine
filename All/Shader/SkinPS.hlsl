#include "LightBuffer.hlsli"

iBaseLight gAbsLight;

struct PixelIn
{
	float4 PosH     : SV_POSITION;
	float3 PosW     : POSITION;
	float3 NormalW  : NORMAL;
	float3 TangentW : TANGENT;
	float2 Tex      : TEXCOORD;
};

SamplerState LinearRepeat :register(s0);


float4 main(PixelIn pin) : SV_TARGET
{
	float4 texColor = float4(1.f, 1.f, 1.f, 1.f);
	texColor = TexDiffuse.Sample(LinearRepeat, pin.Tex);

	//lerp can unnormalize
	pin.NormalW = normalize(pin.NormalW);
	float3 normalMapSample = TexNormalMap.Sample(LinearRepeat, pin.Tex).rgb;
		float3 bumpedNormalW = NormalSampleCalc(normalMapSample, pin.NormalW, pin.TangentW);

		float3 v = normalize(gEyePosW - pin.PosW);

		float4 ambient = gAbsLight.Ambient(bumpedNormalW)*gMat.ambient;
		float4 diffuse = gAbsLight.Diffuse(bumpedNormalW, pin.PosW, v)*gMat.diffuse;
		float4 spec = gAbsLight.Specular(bumpedNormalW, gMat.specular.w, pin.PosW, v)*gMat.specular;

		float4 litColor = texColor*(ambient + diffuse) + spec;
		litColor.a = texColor.a;
	return litColor;
}