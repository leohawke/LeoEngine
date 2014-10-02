cbuffer  cbMatrix{
	float4x4 gViewProj;
	float2 gOffset;
	float2 gUVScale;
};

Texture2D<float> gHeightMap : register(t0);
SamplerState gClampLinear: register(s0);

struct VertexIn
{
	uint2 pos:POSITION;
};

float4 main(VertexIn vin) : SV_POSITION
{
	float2 newpos = f16tof32(vin.pos) + gOffset;
	float2 uv = newpos*gUVScale;
	float3 heightpos = float3(newpos.x, gHeightMap.SampleLevel(gClampLinear, uv,0).x, newpos.y);
	float4 pos = float4(heightpos, 1.f);
	return mul(pos, gViewProj);
}