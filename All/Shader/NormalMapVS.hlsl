cbuffer cbChangerEveryFrame : register(b0)
{
	matrix World;
	matrix WorldInvTranspose;
	matrix WorldViewProj;
	matrix ShadowViewProjTex;
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
	float4 Shadow_PosH : POSITION1;
	float3 PosW     : POSITION0;
	float3 NormalW  : NORMAL;
	float3 TangentW : TANGENT;
	float2 Tex      : TEXCOORD;
};

VertexOut main(VertexIn vin)
{
	VertexOut vout;

	// Transform to world space space.
	vout.PosW = mul(float4(vin.PosL, 1.0f), World).xyz;
	vout.NormalW = mul(vin.NormalL, (float3x3)WorldInvTranspose);
	vout.TangentW = mul(vin.TangentL, (float3x3)World);

	// Transform to homogeneous clip space.
	vout.PosH = mul(float4(vin.PosL, 1.0f),WorldViewProj);
	vout.Shadow_PosH = mul(float4(vout.PosW, 1.f), ShadowViewProjTex);
	// Output vertex attributes for interpolation across triangle.
	vout.Tex = vin.Tex;

	return vout;
}