cbuffer cbChangerEveryFrame : register(b0)
{
	matrix InvTransposeWorldView;
	matrix WorldViewProj;
}

struct VertexIn
{
	float3 PosL     : POSITION;
	float3 NormalL  : NORMAL;
	float2 Tex      : TEXCOORD;
	float3 TangentL : TANGENT;
};

struct VertexOut
{
	float4 PosH     : SV_POSITION;
	half3 NormalV  : NORMAL;
	float2 Tex      : TEXCOORD;
};

VertexOut main(VertexIn vin)
{
	VertexOut vout;

	vout.NormalV = half3(mul(vin.NormalL, (float3x3)InvTransposeWorldView));
	vout.PosH = mul(float4(vin.PosL, 1.0f), WorldViewProj);
	vout.Tex = vin.Tex;

	return vout;
}