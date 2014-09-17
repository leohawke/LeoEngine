
Texture2D gInputTexture :register(t0);
SamplerState ClampLinear : register(s0)
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = Clamp;
	AddressV = Clamp;
};

struct DeformVertex
{
	float4 pos : SV_Position;
	float2 texCoord : TEXCOORD1;
};

float4 GradientPS(DeformVertex input) : SV_Target
{
	input.texCoord.y = 1 - input.texCoord.y;
	float x0 = gInputTexture.Sample(ClampLinear, input.texCoord, int2(1, 0)).x;
	float x1 = gInputTexture.Sample(ClampLinear, input.texCoord, int2(-1, 0)).x;
	float y0 = gInputTexture.Sample(ClampLinear, input.texCoord, int2(0, 1)).x;
	float y1 = gInputTexture.Sample(ClampLinear, input.texCoord, int2(0, -1)).x;
	return float4(x0 - x1, y0 - y1, 0, 0);
}