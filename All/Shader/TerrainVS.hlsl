cbuffer  cbMatrix{
	float4x4 gViewProj;
	float3 gOffset;
};

Texture2D<float> gHeightMap : register(t0);
SamplerState gClampLinear: register(s1);

struct VertexIn
{
	half2 pos:POSITION;
};

float4 main(VertexIn vin) : SV_POSITION
{
	float4 pos = float4(float3(vin.pos.x, 0.f, vin.pos.y)+gOffset, 1.f);
	return mul(pos, gViewProj);
}