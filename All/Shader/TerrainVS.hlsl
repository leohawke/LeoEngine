

cbuffer  cbMatrix{
	float4x4 gViewProj;
	float2 gOffset;
	float2 gUVScale;
};

Texture2D gNoiseTexure : register(t0);
SamplerState RepeatPoint:register(s0)
{
	Filter = MIN_MAG_MIP_POINT;
	AddressU = Wrap;
	AddressV = Wrap;
};

#include "Noise.hlsli"

struct VertexIn
{
	uint2 pos:POSITION;
};

struct VertexOut
{
	float4 PosH : SV_POSITION;
	float2 Tex : TEXCOORD;
};

VertexOut main(VertexIn vin)
{
	float2 newpos = f16tof32(vin.pos) + gOffset;
		float2 uv = newpos*gUVScale + float2(0.5f, 0.5f);
		uv.y = 1.f - uv.y;
	float NoiseScale = 5.f;
#if 0
	float y = hybridTerrain(NoiseScale*uv, int3(3, 3, 1))-0.5f;
#endif
	float y = inoise(NoiseScale*uv);
	y *= 5;
	VertexOut vout;
	vout.PosH = mul(float4(newpos.x, y, newpos.y, 1.f), gViewProj);
	vout.Tex = uv;
	return vout;
}