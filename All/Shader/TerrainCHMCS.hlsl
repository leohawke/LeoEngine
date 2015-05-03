

Texture2D gNoiseTexure : register(t0);
SamplerState RepeatPoint:register(s0)
{
	Filter = MIN_MAG_MIP_POINT;
	AddressU = Wrap;
	AddressV = Wrap;
};

RWTexture2D<half3> Output:register(u0);

cbuffer cbCHMCS {
	float4 gParam;//x,y=>UVScale,z=>NoiseScale,w=>HeightScale
};

groupshared float Height[32 * 32];


#include "Noise.hlsli"
[numthreads(32,32, 1)]
void main( uint3 DTid : SV_DispatchThreadID,uint2 GTid:SV_GroupThreadID )
{
	float2 uv = DTid.xy * gParam.xy;
	//Calc UV
	float y = inoise(gParam.z*uv) * gParam.w;

	Height[GTid.y * 32 + GTid.x] = y;

	GroupMemoryBarrierWithGroupSync();


	int2 sGTid = GTid;

	int2 ri[4] = {
		clamp(sGTid+ int2(-1, -1),int2(0,0),int2(31,31)),//top-left00
		clamp(sGTid + int2(0, -1),int2(0,0),int2(31,31)),//top-middle10
		clamp(sGTid + int2(1, -1),int2(0,0),int2(31,31)),//top-right20
		clamp(sGTid + int2(-1, 0),int2(0,0),int2(31,31))//middle-left01
	};

	int2 gi[4] = {
		clamp(sGTid + int2(-1, -1),int2(0,0),int2(31,31)),//middle-right21
		clamp(sGTid + int2(0, -1),int2(0,0),int2(31,31)),//top-middle02
		clamp(sGTid + int2(1, -1),int2(0,0),int2(31,31)),//top-right12
		clamp(sGTid + int2(-1, 0),int2(0,0),int2(31,31))//middle-left22
	};

	float4 r = { Height[ri[0].y * 32 + ri[0].x],
		Height[ri[1].y * 32 + ri[1].x],
		Height[ri[2].y * 32 + ri[2].x],
		Height[ri[3].y * 32 + ri[3].x] };

	float4 g = { Height[gi[0].y * 32 + gi[0].x],
		Height[gi[1].y * 32 + gi[1].x],
		Height[gi[2].y * 32 + gi[2].x],
		Height[gi[3].y * 32 + gi[3].x] };

	//[ r.x r.y r.z ]   [ 1  2  1 ]
	//[ r.w  0  g.x ] * [ 0  0  0 ] = dx
	//[ g.y g.z g.w ]   [-1 -2 -1 ]
	//http://en.wikipedia.org/wiki/Sobel_operator
	float dx = r.x - r.z + 2.f*r.w - 2.f*g.x + g.y - g.w;
	//[ r.x r.y r.z ]   [ 1  0  -1 ]
	//[ r.w  0  g.x ] * [ 2  0  -2 ] = dy
	//[ g.y g.z g.w ]   [ 1 -0  -1 ]
	float dy = r.x + 2.f*r.y + r.z - g.y - 2.f*g.z - g.w;

	float dz = 0.01f*sqrt(max(0.f, 1.f - dx*dx - dy*dy));

	half3 Normal = normalize(half3(2.f*dx, dz, 2.f*dy));

	Output[DTid.xy] = Normal;
}