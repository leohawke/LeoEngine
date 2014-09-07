cbuffer cbChangeEveryCamera : register(b0)
{
	float3 gEyePos;
	matrix gViewProj;
};

struct VertexIn
{
	float4 PosL : POSITION;
};

struct VertexOut
{
	float4 PosH : SV_POSITION;
	float3 Tex : TEXCOORD;
};

VertexOut main(VertexIn vin)
{
	VertexOut vout;

	//post
	vout.PosH = vin.PosL;

	vout.Tex = mul(vin.PosL,gViewProj).xyz;

	return vout;
}