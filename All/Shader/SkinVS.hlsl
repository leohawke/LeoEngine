cbuffer cbChangerEveryFrame : register(b0)
{
	matrix gWorld;
	matrix gWorldInvTranspose;
	matrix gWorldViewProj;
}

struct VertexIn
{
	float3 PosL     : POSITION;
	float3 NormalL  : NORMAL;
	float2 Tex      : TEXCOORD;
	float3 TangentL : TANGENT;
	uint mJointIndices : JOINTINDICES;
	float3 mWeights : JOINTWEIGHTS;
};

struct VertexOut
{
	float4 PosH     : SV_POSITION;
	float3 PosW     : POSITION;
	float3 NormalW  : NORMAL;
	float3 TangentW : TANGENT;
	float2 Tex      : TEXCOORD;
};

cbuffer SkinMatrix : register(b1){
	float4x4 gSkinMatrix[96];
}

uint4 CalcIndices(uint indices){
	return uint4((indices & 0xff000000) >> 24,
		(indices & 0x00ff0000) >> 16,
		(indices & 0x0000ff00) >> 8,
		(indices & 0x000000ff)
		);
}



VertexOut main(VertexIn vin)
{
	VertexOut vout;
	float4 weights= float4(vin.mWeights, 1.f - (vin.mWeights.x + vin.mWeights.y, +vin.mWeights.z));
	uint4 indices = uint4(CalcIndices(vin.mJointIndices));

	float3 posL= float3(0.f, 0.f, 0.f);
	float3 normalL = float3(0.0f, 0.0f, 0.0f);
	float3 tangentL = float3(0.0f, 0.0f, 0.0f);
	for (int i = 0; i < 4; ++i){
		posL += weights.x* mul(vin.PosL, (float3x3)gSkinMatrix[indices.x]).xyz;
		normalL += weights[i] * mul(vin.NormalL, (float3x3)gSkinMatrix[indices.x]);
		tangentL += weights[i] * mul(vin.TangentL.xyz, (float3x3)gSkinMatrix[indices.x]);
		indices = indices.yzwx;
		weights = weights.yzwx;
	}

	// Transform to world space space.
	vout.PosW = mul(float4(posL, 1.0f), gWorld).xyz;
	vout.NormalW = mul(normalL, (float3x3)gWorldInvTranspose);
	vout.TangentW = mul(tangentL, (float3x3)gWorld);
	vout.Tex = vin.Tex;
	// Transform to homogeneous clip space.
	vout.PosH = mul(float4(posL, 1.0f), gWorldViewProj);
	return vout;
}