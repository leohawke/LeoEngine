#include "LightHelper.hlsli"

cbuffer cbPerFrame : register(b0)
{
	DirectionLight gDirLight;
	PointLight	   gPoiLight;
	SpotLight	   gSpoLight;
}

cbuffer cbPerPrimitive : register(b1)
{
	Material gMat;
}

cbuffer cbPerView : register(b2)
{
	float3 gEyePosW;
	float pad;
}

Texture2D TexDiffuse :register(t0);
Texture2D TexNormalMap : register(t1);
Texture2D ShadowMap : register(t2);