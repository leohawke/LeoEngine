#include <NormalUti>

Texture2D<float4> texNormalAlpha:register(t0);

SamplerState samPoint : register(s0);


float4 main(float2 tc0 : TEXCOORD0) : SV_TARGET
{
	return float4(DeCompressionNormal(texNormalAlpha.Sample(samPoint,tc0).rgb), 1.0f);
}