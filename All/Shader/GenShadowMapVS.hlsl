cbuffer perModel : register(b0) {
	matrix mWorld;
}

cbuffer perLight : register(b1) {
	matrix mViewProj;
}


float4 main( float3 pos : POSITION ) : SV_POSITION
{
	return mul(mul(float4(pos,1.f),mWorld),mViewProj);
}