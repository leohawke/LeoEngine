cbuffer cbColor : register(b0)
{
	float4 gColor;
};

float4 main(float4 posh : SV_POSITION) : SV_TARGET
{
	return gColor;
}