#include <NormalUti>
#include <Lighting>
Texture2D<float> texDepth :register(t0);
Texture2D<float4> texNormalAlpha:register(t1);

SamplerState samPoint : register(s0);

struct AmbientLight {
	float3 Directional;
	float attrib_x;
	float3 Diffuse;
	float attrib_y;
};

cbuffer LightParam:register(b0) {
	AmbientLight Light;
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
	float4 lighting = 0;

	return CalcColor(0.5f+0.5f*dot(dir,normal), 0, 1, light_color);
}