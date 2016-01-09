cbuffer cbChangerEveryFrame : register(b0)
{
	matrix World;
	matrix InvTransposeWorld;
	matrix WorldViewProj;
}

cbuffer SkinMatrix : register(b1) {
	matrix SkinMatrix[96];
}

struct VertexIn
{
	float3 PosL     : POSITION;
	float3 NormalL  : NORMAL;
	float2 Tex      : TEXCOORD;
	float4 TangentL : TANGENT;
	uint mJointIndices : JOINTINDICES;
	float3 mWeights : JOINTWEIGHTS;
};

struct VertexOut
{
	float3 NormalW  : NORMAL;
	float4 TangentW : TANGENT;
	float2 Tex      : TEXCOORD;
	float4 PosH     : SV_POSITION;
};



uint4 CalcIndices(uint indices) {
	return uint4(
		(indices & 0xff000000) >> 24,
		(indices & 0x00ff0000) >> 16,
		(indices & 0x0000ff00) >> 8,
		(indices & 0x000000ff)
		);
}



VertexOut main(VertexIn vin)
{
	VertexOut vout;

	float4 weights = float4(vin.mWeights, 1.f - vin.mWeights.x - vin.mWeights.y - vin.mWeights.z);

	uint4 indices = CalcIndices(vin.mJointIndices);

	float3 posL = float3(0.f, 0.f, 0.f);
	float3 normalL = float3(0.0f, 0.0f, 0.0f);
	float3 tangentL = float3(0.0f, 0.0f, 0.0f);
	[unroll]
	for (int i = 0; i < 4; ++i) {
		posL += weights[i] * mul(float4(vin.PosL, 1.f), SkinMatrix[indices[i]]).xyz;
		normalL += weights[i] * mul(vin.NormalL, (float3x3)SkinMatrix[indices[i]]);
		tangentL += weights[i] * mul(vin.TangentL.xyz, (float3x3)SkinMatrix[indices[i]]);
	}

	// Transform to world space space.
	vout.NormalW =normalize(mul(normalL, (float3x3)InvTransposeWorld));
	vout.TangentW =float4(normalize(mul(tangentL, (float3x3)World)),vin.TangentL.w);
	vout.Tex = vin.Tex;
	// Transform to homogeneous clip space.
	vout.PosH = mul(float4(posL, 1.0f),WorldViewProj);
	return vout;
}