#include "common.hlsli"
cbuffer  cbMatrix{
	float4x4 gViewProj;
	float2 gOffset;
	float2 gUVScale;
};


Texture2D<half> gHeightMap : register(t0);

SamplerState ClampPoint:register(s0)
{
	Filter = MIN_MAG_MIP_POINT;
	AddressU = Clamp;
	AddressV = Clamp;
};


struct VertexIn
{
	uint2 pos:POSITION;
};

struct VertexOut
{
	float4 PosH : SV_POSITION;
	float2 Tex : TEXCOORD;
	half3 NormalV :NORMAL;
};

VertexOut main(VertexIn vin)
{
	float2 newpos = f16tof32(vin.pos) + gOffset;
	float2 uv = newpos*gUVScale + float2(0.5f, 0.5f);
	uv.y = 1.f - uv.y;
	
	float y = gHeightMap.SampleLevel(ClampPoint, uv, 0);

	VertexOut vout;
	vout.PosH = mul(float4(newpos.x, y, newpos.y, 1.f), gViewProj);
	vout.Tex = uv;
	vout.NormalV = Sobel(uv, gHeightMap, ClampPoint);
	return vout;
}