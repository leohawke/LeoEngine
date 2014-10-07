

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

float4 main(VertexIn vin) : SV_POSITION
{
	float2 newpos = f16tof32(vin.pos) + gOffset;
	float2 uv = newpos*gUVScale + float2(0.5f, 0.5f);
	float y =hybridTerrain(uv, int3(3,3,1));
	float3 heightpos = float3(newpos.x,y , newpos.y);
	float4 pos = float4(heightpos, 1.f);
	return mul(pos, gViewProj);
}