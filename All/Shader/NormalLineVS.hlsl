cbuffer cbChangerEveryModel : register(b0)
{
	matrix World;
	matrix WorldInvTranspose;
}

struct VertexIn
{
	float3 PosL     : POSITION;
	float3 NormalL  : NORMAL;
};

struct VertexOut
{
	float3 PosW     : POSITION;
	float3 NormalW  : NORMAL;
};

VertexOut main(VertexIn vin)
{
	VertexOut vout;

	// Transform to world space space.
	vout.PosW = mul(float4(vin.PosL, 1.0f), World).xyz;
	vout.NormalW = normalize(mul(vin.NormalL, (float3x3)WorldInvTranspose).xyz);


	return vout;
}