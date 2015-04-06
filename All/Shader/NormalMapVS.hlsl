cbuffer cbChangerEveryFrame : register(b0)
{
	matrix WorldView;
	matrix WorldInvTransposeView;
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
	half3 NormalV  : NORMAL;
	float2 Tex      : TEXCOORD;
	float3 PosV     : POSITION;
};

VertexOut main(VertexIn vin)
{
	VertexOut vout;

	// Transform to view space space.
	vout.PosV = mul(float4(vin.PosL, 1.0f), WorldView).xyz;
	vout.NormalV =half3(mul(vin.NormalL, (float3x3)WorldInvTransposeView));
	//vout.TangentV = mul(vin.TangentL, (float3x3)WorldView);

	// Transform to homogeneous clip space.
	vout.PosH = mul(float4(vin.PosL, 1.0f),WorldViewProj);
	//vout.Shadow_PosH = mul(float4(vout.PosW, 1.f), ShadowViewProjTex);
	//vout.Shadow_PosH = float4(1.f, 1.f, 1.f, 1.f);
	// Output vertex attributes for interpolation across triangle.
	vout.Tex = vin.Tex;

	return vout;
}