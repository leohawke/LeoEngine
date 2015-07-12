
cbuffer  cbMatrix{
	float4x4 gViewProj;
	float4x4 gView;
	float2 gOffset;
	float2 gUVScale;
};


Texture2D<float> gHeightMap : register(t0);

SamplerState ClampPoint:register(s0);


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
	
	float y = gHeightMap.SampleLevel(ClampPoint, uv, 0);

	VertexOut vout;
	vout.PosH = mul(float4(newpos.x, y, newpos.y, 1.f), gViewProj);
	vout.Tex = uv;
	return vout;
}