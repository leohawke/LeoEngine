#ifdef DEBUG
cbuffer LodColor
{
	float4 gColor;
};
#endif

float4 main(float4 pos : SV_POSITION) : SV_TARGET
{
	return gColor;
}