struct VertexIn
{
	float4 PosH : POSITION;
	float3 ToFarPlane :TEXCOORD0;
	float2 Tex : TEXCOORD1;
};

struct VertexOut
{
	float4 PosH : SV_POSITION;
	float3 ToFarPlane :TEXCOORD0;
	float2 Tex : TEXCOORD1;
};

VertexOut main(VertexIn vin)
{
	VertexOut vout;
	vout.PosH = vin.PosH;
	vout.ToFarPlane = vin.ToFarPlane;
	vout.Tex = vin.Tex;
	return vout;
}