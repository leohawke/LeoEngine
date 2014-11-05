cbuffer cbChangerEveryFrame : register(b0)
{
	matrix gWorld;
	matrix gWorldInvTranspose;
	matrix gWorldViewProj;
}

cbuffer SkinMatrix : register(b1){
	matrix gSkinMatrix[96];
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
#if 0
	float weights[4];
	weights[0] = vin.mWeights.x;
	weights[1] = vin.mWeights.y;
	weights[2] = vin.mWeights.z;
	weights[3] = 1.f - weights[0] - weights[1] - weights[2];
#else
	float4 weights = float4(vin.mWeights, 1.f - vin.mWeights.x - vin.mWeights.y - vin.mWeights.z);
#endif
		uint4 indices = CalcIndices(vin.mJointIndices);

		float3 posL = float3(0.f, 0.f, 0.f);
		float3 normalL = float3(0.0f, 0.0f, 0.0f);
		float3 tangentL = float3(0.0f, 0.0f, 0.0f);
		[unroll]
		for (int i = 0; i < 4; ++i){
			posL += weights[i] * mul(float4(vin.PosL, 1.f), gSkinMatrix[indices[i]]).xyz;
			normalL += weights[i] * mul(vin.NormalL, (float3x3)gSkinMatrix[indices[i]]);
			tangentL += weights[i] * mul(vin.TangentL, (float3x3)gSkinMatrix[indices[i]]);
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