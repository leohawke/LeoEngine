cbuffer cbChangeOnSet : register(b0)
{
	float4 gColor;
}

struct GSOutput
{
	float4 PosH : SV_POSITION;
};

float4 main(GSOutput pin) : SV_TARGET
{
	return gColor;
}