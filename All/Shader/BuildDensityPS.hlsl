struct GeoOut
{
	float3 PosW	: POSITION;
	float4 PosH	: SV_POSITION;
	uint Index	: SV_RenderTargetArrayIndex;
};

#include "leoDensity.hlsli"

float main(GeoOut pin) : SV_Target
{
#if 1
	float ret = density(pin.PosW);
#else
	float ret = -pin.PosW.y;
#endif
	return ret;
}