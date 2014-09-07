cbuffer CBColor : register(b0)
{
	float4 color;
}

struct PixelIn
{
	float4 PosH : SV_POSITION;
};

float4 main(PixelIn pin) : SV_TARGET
{
	return color;
}