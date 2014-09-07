cbuffer cbChangeEveryCamera : register(b0)
{
	matrix gViewProj;
};


float4 main( float4 pos : POSITION ) : SV_POSITION
{
	return mul(pos,gViewProj);
}