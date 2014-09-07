struct VertexIn
{
	float4 Pos : POSITION;
	float2 Tex : TEXCOORD;
};

struct VertexOut
{
	float4 PosH : SV_POSITION;
	float2 Tex : TEXCOORD;
};

VertexOut main(VertexIn vin)
{
	VertexOut vout;
	vout.PosH = vin.Pos;
	vout.Tex = vin.Tex;
	return vout;
}