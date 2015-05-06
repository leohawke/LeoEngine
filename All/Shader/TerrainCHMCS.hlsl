

Texture2D gNoiseTexure : register(t0);
SamplerState RepeatPoint:register(s0)
{
	Filter = MIN_MAG_MIP_POINT;
	AddressU = Wrap;
	AddressV = Wrap;
};

RWTexture2D<float> Output:register(u0);

cbuffer cbCHMCS :register(c0){
	float4 gParam;//x,y=>UVScale,z=>NoiseScale,w=>HeightScale
};


#include "Noise.hlsli"
[numthreads(32,32, 1)]
void main( uint3 DTid : SV_DispatchThreadID,uint2 GTid:SV_GroupThreadID )
{
	float2 uv = DTid.xy * gParam.xy;
	//Calc UV
	float y = inoise(gParam.z*uv)*gParam.w;


	Output[DTid.xy] = y;
}