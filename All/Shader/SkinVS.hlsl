cbuffer PerSkelton : register(b0){
	float4x4 gWorldViewProj;
};

cbuffer SkinMatrix : register(b1){
	float4x4 gSkinMatrix[96];
}

struct VertexIn{
	float3 mPos : POSITION;
	float3 mNormal : NORMAL;
	float2 mTex : TEXCOORD;
	uint mJointIndices : JOINTINDICES;
	float3 mWeights : JOINTWEIGHTS;
};

uint4 CalcIndices(uint indices){
	return uint4((indices & 0xff000000) >> 24,
		(indices & 0x00ff0000) >> 16,
		(indices & 0x0000ff00) >> 8,
		(indices & 0x000000ff)
		);
}



float4 main(VertexIn vin) : SV_POSITION
{
	float4 weights= float4(vin.mWeights, 1.f - (vin.mWeights.x + vin.mWeights.y, +vin.mWeights.z));
	uint4 indices = uint4(CalcIndices(vin.mJointIndices));

	float3 posL= float3(0.f, 0.f, 0.f);
	for (int i = 0; i < 4; ++i){
		posL +=weights.x* mul(float4(vin.mPos,1.f),gSkinMatrix[indices.x]).xyz;
		indices = indices.yzwx;
		weights = weights.yzwx;
	}
	return mul(float4(posL,1.f),gWorldViewProj);
}