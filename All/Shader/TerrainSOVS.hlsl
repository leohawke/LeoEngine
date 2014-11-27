cbuffer cbSOVS {
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
	uint Id : SV_VERTEXID;
};

struct VertexOut {
	float H : HEIGHT;
	uint Id : ID;
};

VertexOut main(VertexIn vin)
{
	float2 newpos = f16tof32(vin.pos) + gOffset;
	float2 uv = newpos*gUVScale + float2(0.5f, 0.5f);
	uv.y = 1.f - uv.y;
	float NoiseScale = 5.f;
	VertexOut vout;
	vout.H = inoise(NoiseScale*uv)*5;
	vout.Id = vin.Id;
	return vout;
}