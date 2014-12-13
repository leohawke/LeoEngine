cbuffer perLight : register(b0) {
	matrix mViewProj;
}


float4 main( float3 pos : POSITION ) : SV_POSITION
{
	return mul(float4(pos,1.f),mViewProj);
}